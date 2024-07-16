#include "ErogationController.h"

ErogationController::ErogationController() {}

void ErogationController::init(Preferences *preferencePointer, BLE_controller *bleControllerPointer, ESP32Time *rtcPointer)
{
    this->bleControllerPointer = bleControllerPointer;
    this->preferencePointer = preferencePointer;
    this->rtcPointer = rtcPointer;

    pinMode(EROGATION_PIN, OUTPUT);
    digitalWrite(EROGATION_PIN, false);

    preferencePointer->begin("configuration", false);

    // controllo se il database deve essere inizializzato
    // se non è inzializzato allora lo inizializzo, altrimenti lo leggo e basta
    if (preferencePointer->isKey("initialized") == false)
    {
        Serial.println("Creating new configuration data");
        saveConfiguration(true);
    }

    probabilityThreshold = preferencePointer->getInt("prob_TH");
    forecastHoursAhead = preferencePointer->getInt("foreCastAhead");
    forecastHoursPast = preferencePointer->getInt("foreCastPast");
    weatherCheckOffset = preferencePointer->getInt("checkOffset");
    duration = preferencePointer->getInt("duration");
    dailyStart = preferencePointer->getInt("dailyStart", 0);

    // Impostazione dei valori a disposizione del BLE
    bleControllerPointer->setConfigurationParamter(probabilityThreshold, forecastHoursAhead, forecastHoursPast, weatherCheckOffset);
    bleControllerPointer->setErogationDuration(duration);
    bleControllerPointer->setStartTime(dailyStart);
    bleControllerPointer->setErogationRemainingTime(0);

    Serial.println("init done");
}

void ErogationController::debugPrintConfiguration()
{
    Serial.print("\n╔════════════════╦═══════╗\n");
    Serial.print("║ Prob Treshold  ║");
    Serial.printf("%6d ║\n", probabilityThreshold);
    Serial.print("║ Forecast Ahead ║");
    Serial.printf("%6d ║\n", forecastHoursAhead);
    Serial.print("║ Forecast Past  ║");
    Serial.printf("%6d ║\n", forecastHoursPast);
    Serial.print("║ Offset         ║");
    Serial.printf("%6d ║\n", weatherCheckOffset);
    Serial.print("║ Duration       ║");
    Serial.printf("%6d ║\n", duration);
    Serial.print("║ Start time     ║");
    Serial.printf("%6d ║\n", dailyStart);
    Serial.print("╚════════════════╩═══════╝\n\n");
}

/**
 * @brief Salva i dati di configurazione in memoria usando le Preferences quindi in memoria permamente
 *
 * @param useDefault Se abilitato salva dei valori di default.
 * Consigliato quando si sta creando il dataabse o lo si sta resettando
 */
void ErogationController::saveConfiguration(bool useDefault)
{

    if (useDefault)
    {
        preferencePointer->putBool("initialized", true);
        preferencePointer->putInt("prob_TH", 50);
        preferencePointer->putInt("foreCastAhead", 2);
        preferencePointer->putInt("foreCastPast", 2);
        preferencePointer->putInt("checkOffset", 30 * SECOND_IN_MINUTE);
        preferencePointer->putInt("duration", 5 * SECOND_IN_MINUTE);
        preferencePointer->putInt("dailyStart", 0);
        return;
    }

    preferencePointer->putInt("prob_TH", probabilityThreshold);
    preferencePointer->putInt("foreCastAhead", forecastHoursAhead);
    preferencePointer->putInt("foreCastPast", forecastHoursPast);
    preferencePointer->putInt("checkOffset", weatherCheckOffset);
    preferencePointer->putInt("duration", duration);
    preferencePointer->putInt("dailyStart", dailyStart);

    Serial.println("saved new paraemter");
    debugPrintConfiguration();
}

void ErogationController::startErogation()
{
    Serial.println("erogation start");
    erogationStatus = 1;
    endErogationTime = now + duration;
    bleControllerPointer->setErogationRemainingTime(endErogationTime);

    digitalWrite(EROGATION_PIN, HIGH);
    updateStatus();
}

void ErogationController::stopErogation()
{
    Serial.print(now);
    Serial.println(": erogation stopped");
    endErogationTime = -1;
    bleControllerPointer->setErogationRemainingTime(endErogationTime);
    erogationStatus = 0;
    digitalWrite(EROGATION_PIN, LOW);
    updateStatus();
}

void ErogationController::updateStatus()
{
    bleControllerPointer->updateErogationStatus(erogationStatus);
}

void ErogationController::syncronizeTime()
{
    now = rtcPointer->getHour(true) * SECOND_IN_HOUR + rtcPointer->getMinute() * SECOND_IN_MINUTE + rtcPointer->getSecond();
}

/**
 * @brief Funzione principale per il controllo delle eorgazioni.
 * - Gestisce il controllo di eventuali erogazioni programmate
 * - Effettua la chimata API al meteo per controllare se avviare l'erogazione
 * - Atttiva o disattiva l'eorgazione
 */
void ErogationController::handleErogation()
{

    if (now == dailyStart)
    {
        JsonDocument doc;
        apiController.requestWeatherData(&doc);

        int hourNow = rtcPointer->getHour(true);
        bool overThreshhold = false;

        Serial.println("API request done");
        Serial.print("checking ");
        Serial.print(forecastHoursPast);
        Serial.print("in the past and ");
        Serial.print(forecastHoursAhead);
        Serial.println("in the future");

        Serial.print("Probability setted to");
        Serial.println(probabilityThreshold);

        for (int i = hourNow - forecastHoursPast; i <= hourNow + forecastHoursAhead; i++)
        {
            if (i >= 24 || i < 0)
                continue;

            if (doc["hourly"]["precipitation_probability"][i] >= probabilityThreshold)
            {
                overThreshhold = true;
            }
        }

        // Se la probabilità di precipitazione supera il valore di threshold allora l'erogazione
        // è annullata, viene salvata e viene esguito un nuovo controllo sulle prossime erogazioni
        if (overThreshhold)
        {
            Serial.println("too much rain porbbaility. Erogation aborted");
            return;
        }

        Serial.println("Not enought rain probaility to stop erogation. Erogation started");

        // altrimenti l'erogazione è avviata e viene salvato il moento di stop
        startErogation();

        Serial.print(now);
        Serial.println(": erogation started");
        return;
    }

    if (endErogationTime < 0)
        return;

    bleControllerPointer->setErogationRemainingTime(endErogationTime - now);

    if (now > endErogationTime)
    {
        stopErogation();

        return;
    }
}

void ErogationController::setStartTime(uint32_t newStartTime)
{
    Serial.print("New startTime: ");
    Serial.println(newStartTime);
    dailyStart = newStartTime;
    bleControllerPointer->setStartTime(newStartTime);
    saveConfiguration(false);
}

void ErogationController::updateConfigurationValue(ConfigurationParameter parameter, uint16_t value)
{
    Serial.println("New configuration value:");
    Serial.print("Parameter: ");
    Serial.println(parameter);
    Serial.print("Value: ");
    Serial.println(value);
    switch (parameter)
    {
    case PROBABILITY:
        probabilityThreshold = (uint8_t)value;
        break;
    case FORECAST_AHEAD:
        forecastHoursAhead = (uint8_t)value;
        break;
    case FORECAST_PAST:
        forecastHoursPast = (uint8_t)value;
        break;
    case WEATHER_CHECK:
        weatherCheckOffset = (uint16_t)value;
        break;

    case DAILY_START:
        dailyStart = value;
        break;
    }

    bleControllerPointer->setConfigurationParamter(probabilityThreshold, forecastHoursAhead, forecastHoursPast, weatherCheckOffset);
    saveConfiguration(false);
}

void ErogationController::setDuration(uint16_t newDuration)
{
    Serial.print("New Duration: ");
    Serial.println(newDuration);
    duration = newDuration;
    bleControllerPointer->setErogationDuration(duration);
    saveConfiguration(false);
}
