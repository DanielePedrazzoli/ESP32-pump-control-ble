#include <Arduino.h>

#define ARDUINOJSON_SLOT_ID_SIZE 1
#define ARDUINOJSON_STRING_LENGTH_SIZE 1
#define ARDUINOJSON_USE_DOUBLE 0
#define ARDUINOJSON_USE_LONG_LONG 0
#include <ArduinoJson.h>

#include <BLE_controller.h>
#include <Preferences.h>
#include <ConnectionController.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <ErogationController.h>
#include <API_Controller.h>

BLE_controller bleController = BLE_controller();
ConnectionController connectionController = ConnectionController();
ErogationController erogationController = ErogationController();
ApiController apiController = ApiController();
Preferences preferences;

const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
ESP32Time rtc;
uint8_t lastSecond = 0;

/** Handler class for characteristic actions */
class CharacteristicCallbacks : public NimBLECharacteristicCallbacks
{
  void onWrite(NimBLECharacteristic *pCharacteristic)
  {
    if (pCharacteristic->getUUID().equals(bleController.uuidWifiCredential))
    {
      String value = pCharacteristic->getValue();
      connectionController.setCredential(value);
      connectionController.needTryConnection = true;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidControll))
    {
      if (pCharacteristic->getValue()[0] == 1)
      {
        erogationController.startErogation();
      }
      else
      {
        erogationController.stopErogation();
      }
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidDuration))
    {
      if (pCharacteristic->getValue().length() < 2)
        return;
      erogationController.setDuration(pCharacteristic->getValue()[0] | pCharacteristic->getValue()[1] << 8);
      return;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidThreshold))
    {
      erogationController.updateConfigurationValue(ConfigurationParameter::PROBABILITY, pCharacteristic->getValue()[0]);
      return;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidForecastAhead))
    {
      erogationController.updateConfigurationValue(ConfigurationParameter::FORECAST_AHEAD, pCharacteristic->getValue()[0]);
      return;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidForecastPast))
    {
      erogationController.updateConfigurationValue(ConfigurationParameter::FORECAST_PAST, pCharacteristic->getValue()[0]);
      return;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidOffset))
    {
      erogationController.updateConfigurationValue(ConfigurationParameter::WEATHER_CHECK, pCharacteristic->getValue()[0] | pCharacteristic->getValue()[1] << 8);
      return;
    }

    if (pCharacteristic->getUUID().equals(bleController.uuidStartTime))
    {
      erogationController.setStartTime(pCharacteristic->getValue()[0] | pCharacteristic->getValue()[1] << 8 | pCharacteristic->getValue()[2] << 16 | pCharacteristic->getValue()[3] << 24);
      return;
    }
  };
};

CharacteristicCallbacks characteristicCallbacks;

void setupRTC()
{
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov", "time.google.com");
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return;
  }

  rtc.setTimeStruct(timeinfo);
  connectionController.hasInternet(true);
  apiController.init();
}

void setup()
{
  Serial.begin(115200);
  bleController.init(&characteristicCallbacks);
  erogationController.init(&preferences, &bleController, &rtc);

  bool connected = connectionController.init(&preferences, &bleController);

  if (connected)
  {
    setupRTC();
  }
}

void loop()
{

  if (lastSecond != rtc.getSecond())
  {
    lastSecond = rtc.getSecond();
    erogationController.syncronizeTime();
    erogationController.handleErogation();
  }

  // se la scansione delle reti è attiva allora la eseguo e segnalo
  // tramite ble le  reti individuate con la loro potenza
  if (connectionController.isScanning())
  {
    String result = connectionController.scanNetwork();
    bleController.setNetworks(&result);
    delay(2500);
  }

  // se è necessario provare una connessione allora la tento
  if (connectionController.needTryConnection)
  {
    bool connectionSuccess = connectionController.tryConnection();

    if (connectionSuccess)
    {
      setupRTC();
    }
  }
}
