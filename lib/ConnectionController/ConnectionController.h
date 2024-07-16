#ifndef CONNECTION_CONTROLLER
#define CONNECTION_CONTROLLER
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <BLE_controller.h>

#define SEPARATOR_CHAR "?"

enum WIFI_STATUS
{
    CONNECTED = 0,
    SCANNING = 1,
    CONNECTING = 2,
    HAS_INTERNET = 3,
    CONNECTION_FAILED = 4,
};

class ConnectionController
{
private:
    String retriveSSID(String);
    String retrivePassowrd(String);
    void saveCredential();

    Preferences *preferencePointer;
    BLE_controller *bleControllerPointer;

    String ssid;
    String password;

    // bit 0 = connessione, 1 connesso ad una rete wifi, 0 non connesso ad una rete wifi
    // bit 1 = scansione - 1 scansione attiva, 0 scansione wifi disattivata
    // bit 2 = connessione in corso - 1 connessione in corso, 0 nessun tentativo di connessione in corso
    // bit 3 = connettività internet - 1 internet presente, 0 internet non presente
    char wifiConnectionStatus;
    void updateWifiStatus(WIFI_STATUS, bool);
    void sendWifiStatus();

public:
    ConnectionController();

    bool init(Preferences *, BLE_controller *);

    bool tryConnection();
    void setCredential(String, String);
    void setCredential(String);
    String scanNetwork();

    bool credentialExist = false;
    bool needTryConnection = false;

    void hasInternet(bool);

    bool isConnected();
    bool isScanning();
    bool isConnecting();
    bool hasInternet();
};

#endif