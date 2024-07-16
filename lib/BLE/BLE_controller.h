
#ifndef BLE_CONTROLLER
#define BLE_CONTROLLER
#include "NimBLEDevice.h"

#define UUID_SERVICE_CONNECTION "00000000-0001-4181-8813-29504f5dd727"
#define UUID_CHARACTERISTIC_WIFI_CCONNECTION_STATUS "00000001-0001-4018-950b-bd7f813ae149"
#define UUID_CHARACTERISTIC_CREDENTIAL "00000002-0001-4018-950b-bd7f813ae149"
#define UUID_CHARACTERISTIC_WIFI_POWER "00000003-0001-461e-b02d-a253381b64d8"
#define UUID_CHARACTERISTIC_AVAIABLE_NETWORK "00000004-0001-437c-99c4-7b829c179bb9"
#define UUID_CHARACTERISTIC_NETWORK_IP "00000005-0001-435b-86b1-b7fd6559e949"

#define UUID_EROGATION_DATA "00000000-0002-49ea-90a2-93e2b252141c"
#define UUID_CHARACTERISTIC_EROGATION_STATUS "00000001-0002-4ac2-8cc9-decfc9511411"
#define UUID_CHARACTERISTIC_EROGATION_CONTROLL "00000002-0002-4f3a-9de5-cae7c45f5a8c"
#define UUID_CHARACTERISTIC_EROGATION_DURATION "00000003-0002-4f3a-9de5-cae7c45f5a8c"
#define UUID_CHARACTERISTIC_REMAINING_TIME "00000004-0002-4f3a-9de5-cae7c45f5a8c"
#define UUID_CHARACTERISTIC_EROGATION_TIME "00000005-0002-4f3a-9de5-cae7c45f5a8c"

#define UUID_CONFIGURATION_DATA "00000000-0003-49ea-90a2-93e2b252141c"
#define UUID_CHARACTERISTIC_PROBABILITY_THRESHOLD "00000001-0003-4ac2-8cc9-decfc9511411"
#define UUID_CHARACTERISTIC_FORECAST_HOURS_AHEAD "00000002-0003-4f3a-9de5-cae7c45f5a8c"
#define UUID_CHARACTERISTIC_FORECAST_HOURS_PAST "00000003-0003-4f3a-9de5-cae7c45f5a8c"
#define UUID_CHARACTERISTIC_WEATHER_CHECK_OFFSET "00000004-0003-4f3a-9de5-cae7c45f5a8c"

#define UUID_DEVICE_INFO "00000000-0004-49ea-90a2-93e2b252141c"
#define UUID_CHARACTERISTIC_FIRMWARE_VERSION "00000001-0004-4ac2-8cc9-decfc9511411"

class BLE_controller
{
private:
    NimBLEServer *pServer;
    NimBLEService *connectionService;
    NimBLECharacteristic *charWifiCredential;
    NimBLECharacteristic *charPower;
    NimBLECharacteristic *charNetworks;
    NimBLECharacteristic *networkIp;
    // bit 0 = connessione, 1 connesso ad una rete wifi, 0 non connesso ad una rete wifi
    // bit 1 = scansione - 1 scansione attiva, 0 scansione wifi disattivata
    // bit 2 = connessione in corso - 1 connessione in corso, 0 nessun tentativo di connessione in corso
    // bit 3 = connettività internet - 1 internet presente, 0 internet non presente
    // bit 4 = Credenziali presenti ma connessione fallita
    NimBLECharacteristic *wifiConnectionStatus;

    NimBLEService *erogationService;
    NimBLECharacteristic *erogationStatus;
    NimBLECharacteristic *erogationControll;
    NimBLECharacteristic *erogationDuration;
    NimBLECharacteristic *remainingTime;
    NimBLECharacteristic *erogationTime;

    NimBLEService *configurationService;
    NimBLECharacteristic *probabilityThreshold;
    NimBLECharacteristic *forecastHoursAhead;
    NimBLECharacteristic *forecastHoursPast;
    NimBLECharacteristic *weatherCheckOffset;

    NimBLEService *deviceInfoService;
    NimBLECharacteristic *firmwareVersion;

    NimBLEAdvertising *pAdvertising;

    char wifiStatus = 0;

public:
    BLE_controller();

    void init(NimBLECharacteristicCallbacks *);

    void setPower(int);
    void setNetworks(String *);
    void setNetworkIP(String);
    void updateWifiStatus(char);
    void updateErogationStatus(char);
    void setConfigurationParamter(uint8_t, uint8_t, uint8_t, uint16_t);
    void setErogationDuration(uint16_t);
    void setErogationRemainingTime(int32_t);
    void setStartTime(uint32_t);

    NimBLEUUID uuidWifiCredential = NimBLEUUID(UUID_CHARACTERISTIC_CREDENTIAL);
    NimBLEUUID uuidControll = NimBLEUUID(UUID_CHARACTERISTIC_EROGATION_CONTROLL);
    NimBLEUUID uuidDuration = NimBLEUUID(UUID_CHARACTERISTIC_EROGATION_DURATION);
    NimBLEUUID uuidThreshold = NimBLEUUID(UUID_CHARACTERISTIC_PROBABILITY_THRESHOLD);
    NimBLEUUID uuidForecastAhead = NimBLEUUID(UUID_CHARACTERISTIC_FORECAST_HOURS_AHEAD);
    NimBLEUUID uuidForecastPast = NimBLEUUID(UUID_CHARACTERISTIC_FORECAST_HOURS_PAST);
    NimBLEUUID uuidOffset = NimBLEUUID(UUID_CHARACTERISTIC_WEATHER_CHECK_OFFSET);
    NimBLEUUID uuidStartTime = NimBLEUUID(UUID_CHARACTERISTIC_EROGATION_TIME);
};

#endif
