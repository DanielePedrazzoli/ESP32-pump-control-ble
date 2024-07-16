#include <BLE_controller.h>

BLE_controller::BLE_controller() {}

/// @brief
/// @param charPasswordCallback
/// @param charSSIDCallback
/// @param charPumpControllCallback
void BLE_controller::init(NimBLECharacteristicCallbacks *characteristicOnChange)
{
    NimBLEDevice::init("PumpAction");

    pServer = NimBLEDevice::createServer();

    // Conenction service
    connectionService = pServer->createService(UUID_SERVICE_CONNECTION);
    charWifiCredential = connectionService->createCharacteristic(UUID_CHARACTERISTIC_CREDENTIAL, NIMBLE_PROPERTY::WRITE);
    charPower = connectionService->createCharacteristic(UUID_CHARACTERISTIC_WIFI_POWER, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    charNetworks = connectionService->createCharacteristic(UUID_CHARACTERISTIC_AVAIABLE_NETWORK, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::INDICATE);
    networkIp = connectionService->createCharacteristic(UUID_CHARACTERISTIC_NETWORK_IP, NIMBLE_PROPERTY::READ);
    wifiConnectionStatus = connectionService->createCharacteristic(UUID_CHARACTERISTIC_WIFI_CCONNECTION_STATUS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

    charWifiCredential->setCallbacks(characteristicOnChange);

    connectionService->start();
    wifiConnectionStatus->setValue(0);
    charPower->setValue(-1);

    /// erogation service
    erogationService = pServer->createService(UUID_EROGATION_DATA);
    erogationStatus = erogationService->createCharacteristic(UUID_CHARACTERISTIC_EROGATION_STATUS, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    erogationControll = erogationService->createCharacteristic(UUID_CHARACTERISTIC_EROGATION_CONTROLL, NIMBLE_PROPERTY::WRITE);
    erogationDuration = erogationService->createCharacteristic(UUID_CHARACTERISTIC_EROGATION_DURATION, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    remainingTime = erogationService->createCharacteristic(UUID_CHARACTERISTIC_REMAINING_TIME, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    erogationTime = erogationService->createCharacteristic(UUID_CHARACTERISTIC_EROGATION_TIME, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    erogationControll->setCallbacks(characteristicOnChange);
    erogationDuration->setCallbacks(characteristicOnChange);
    erogationTime->setCallbacks(characteristicOnChange);
    erogationStatus->setValue(0);
    erogationService->start();

    /// configuration service
    configurationService = pServer->createService(UUID_CONFIGURATION_DATA);
    probabilityThreshold = configurationService->createCharacteristic(UUID_CHARACTERISTIC_PROBABILITY_THRESHOLD, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    forecastHoursAhead = configurationService->createCharacteristic(UUID_CHARACTERISTIC_FORECAST_HOURS_AHEAD, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    forecastHoursPast = configurationService->createCharacteristic(UUID_CHARACTERISTIC_FORECAST_HOURS_PAST, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    weatherCheckOffset = configurationService->createCharacteristic(UUID_CHARACTERISTIC_WEATHER_CHECK_OFFSET, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    probabilityThreshold->setCallbacks(characteristicOnChange);
    forecastHoursAhead->setCallbacks(characteristicOnChange);
    forecastHoursPast->setCallbacks(characteristicOnChange);
    weatherCheckOffset->setCallbacks(characteristicOnChange);
    configurationService->start();

    // device info service
    deviceInfoService = pServer->createService(UUID_DEVICE_INFO);
    firmwareVersion = deviceInfoService->createCharacteristic(UUID_CHARACTERISTIC_FIRMWARE_VERSION, NIMBLE_PROPERTY::READ);
    firmwareVersion->setValue("0.0.1");
    deviceInfoService->start();

    pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(UUID_SERVICE_CONNECTION);
    pAdvertising->addServiceUUID(UUID_EROGATION_DATA);
    pAdvertising->addServiceUUID(UUID_CONFIGURATION_DATA);
    pAdvertising->addServiceUUID(UUID_DEVICE_INFO);
    pAdvertising->start();
}

void BLE_controller::setPower(int power)
{
    charPower->setValue(power);
    charPower->notify();
}
void BLE_controller::setNetworks(String *networks)
{
    size_t len = networks->length();
    const uint8_t *data = (const uint8_t *)networks->c_str();
    charNetworks->setValue(data, len);
    charNetworks->notify();
}

void BLE_controller::setNetworkIP(String ip)
{
    size_t len = ip.length();
    const uint8_t *data = (const uint8_t *)ip.c_str();
    networkIp->setValue(data, len);
}

void BLE_controller::updateWifiStatus(char newStatus)
{
    wifiConnectionStatus->setValue(newStatus);
    wifiConnectionStatus->notify();
}

void BLE_controller::updateErogationStatus(char newStatus)
{
    erogationStatus->setValue(newStatus);
    erogationStatus->notify();
}

void BLE_controller::setConfigurationParamter(uint8_t prob, uint8_t ahead, uint8_t past, uint16_t offset)
{
    probabilityThreshold->setValue(prob);
    forecastHoursAhead->setValue(ahead);
    forecastHoursPast->setValue(past);
    weatherCheckOffset->setValue(offset);
}

void BLE_controller::setErogationDuration(uint16_t duration)
{
    erogationDuration->setValue(duration);
}

void BLE_controller::setErogationRemainingTime(int32_t time)
{
    remainingTime->setValue(time);
    remainingTime->notify();
}

void BLE_controller::setStartTime(uint32_t time)
{
    erogationTime->setValue(time);
}
