#ifndef EROGATION_CONTROLLER
#define EROGATION_CONTROLLER

#include <BLE_controller.h>
#include <Preferences.h>
#include <ESP32Time.h>
#include <API_Controller.h>

#define EROGATION_PIN 25
#define MAX_EROGATIONS 14

#define SECOND_IN_MINUTE 60
#define SECOND_IN_HOUR SECOND_IN_MINUTE * 60
#define SECOND_IN_DAY SECOND_IN_HOUR * 24
#define SECOND_IN_WEEK SECOND_IN_DAY * 7;

extern ApiController apiController;

enum ConfigurationParameter
{
    PROBABILITY = 0,
    FORECAST_AHEAD = 1,
    FORECAST_PAST = 2,
    WEATHER_CHECK = 3,
    DAILY_START = 4,
};

class ErogationController
{
private:
    void updateStatus();
    // String erogationsToString();
    // void sortErogations();
    void saveConfiguration(bool);
    BLE_controller *bleControllerPointer;
    Preferences *preferencePointer;

    uint8_t erogationStatus = 0;

    uint32_t now;
    int32_t endErogationTime = -1;
    uint8_t nextErogationIndex = -1;

    /** Variabili usate per stabilire l'erogazione */
    // La soglia di probabilita che, se superata nel range di ore [current - lookingBackward, current + lookingForward],
    // porta all'annullamento del'erogazione
    uint8_t probabilityThreshold = 0;

    // Su quante ore nelle previsioni è necessario effettuare il controllo della probabilità
    uint8_t forecastHoursAhead = 0;

    // Su quante ore nella storia è necessario effettuare il controllo della probabilità
    uint8_t forecastHoursPast = 0;

    // Quanto tempo prima dell'erogazione viene effettiuato il controllo del meteo
    uint16_t weatherCheckOffset = 0;

    /** Variabii di eorgazione */
    uint16_t duration = 0;

    uint32_t dailyStart = 0;

    ESP32Time *rtcPointer;

    void debugPrintConfiguration();
    // void debugPrintErogations();
    // void saveErogations();

public:
    ErogationController();

    void init(Preferences *, BLE_controller *, ESP32Time *);
    void startErogation();
    void stopErogation();
    // void calculateNextErogation();
    void handleErogation();
    void syncronizeTime();
    void setDuration(uint16_t);
    void setStartTime(uint32_t);
    // void setErogationValue(uint32_t);
    void updateConfigurationValue(ConfigurationParameter, uint16_t);
};

#endif