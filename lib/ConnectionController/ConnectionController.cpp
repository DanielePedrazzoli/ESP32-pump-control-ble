#include <ConnectionController.h>

ConnectionController::ConnectionController() {}

/**
 * @brief Inizializza la classe e imposta i valori di base oltre a verificare
 * se è possibile una connessione (quindi se le credenziali sono salvate in memoria)
 *
 * Nel caso queste non esistano allora viene abilitata la scansione
 *
 * @param preferencePointer
 */
bool ConnectionController::init(Preferences *preferencePointer, BLE_controller *bleControllerPointer)
{
    this->preferencePointer = preferencePointer;
    this->bleControllerPointer = bleControllerPointer;
    this->preferencePointer->begin("wifiCredential", false);
    credentialExist = preferencePointer->isKey("wifi-credential");

    if (!credentialExist)
    {
        Serial.println("Credenziali non trovate");
        WiFi.mode(WIFI_STA);
        updateWifiStatus(WIFI_STATUS::SCANNING, true);
        sendWifiStatus();
        return false;
    }
    Serial.println("Credenziali trovate");
    setCredential(preferencePointer->getString("wifi-credential", ""));
    tryConnection();
    return true;
}

/**
 * @brief Esegue l'update dello stato in modo più pulito semplificando le operazione bitwise nel codice
 * Non notifica in automatico al bluetooth, questo deve essere effettuato chiamando sendWifiStatus();
 *
 * @param bit Il bit dello stato da modificare
 * @param value Il valore da assegnare al bit indicato
 */
void ConnectionController::updateWifiStatus(WIFI_STATUS bit, bool value)
{
    if (value)
    {
        wifiConnectionStatus |= (1 << bit);
        return;
    }
    wifiConnectionStatus &= ~(1 << bit);
}

/**
 * @brief Notifica al bluetooth il nuovo stato di connessione.
 * Questa funzionalità è stata mantenuta separata poiché nella maggior parte dei casi venivano effettuati
 * due o più cambiamenti di stato in contemporanea e questo avrebbe portato ad un burst di notifiche da
 * parte del bluetooth
 */
void ConnectionController::sendWifiStatus()
{
    bleControllerPointer->updateWifiStatus(wifiConnectionStatus);
}

/**
 * @brief Salva le credenziali in memoria usando la libreria Preferences
 *
 */
void ConnectionController::saveCredential()
{
    String credential = ssid + SEPARATOR_CHAR + password;
    preferencePointer->putString("wifi-credential", credential);
    Serial.print("New credential saved: ");
    Serial.println(credential);
}

/**
 * @brief Recupera la password dalla stringa completa (con ancora il separatore)
 *
 * @param base
 * @return String
 */
String ConnectionController::retrivePassowrd(String base)
{
    char separatorIndex = base.indexOf(SEPARATOR_CHAR);
    return base.substring(separatorIndex + 1);
}

/**
 * @brief Recupera il valore di ssid dalla stringa completa (con ancora il separatore)
 *
 * @param base
 * @return String
 */
String ConnectionController::retriveSSID(String base)
{
    char separatorIndex = base.indexOf(SEPARATOR_CHAR);
    return base.substring(0, separatorIndex);
}

void ConnectionController::setCredential(String ssid, String password)
{
    this->ssid = ssid;
    this->password = password;
}

void ConnectionController::setCredential(String credential)
{
    ssid = retriveSSID(credential);
    password = retrivePassowrd(credential);
}

/**
 * @brief Tenta una connessione usando le variabili presenti in memoria
 *
 * @return Bool true se la connessione è riuscita, false altrimenti
 */
bool ConnectionController::tryConnection()
{
    needTryConnection = false;
    long prevMillis = millis();
    WiFi.begin(ssid.c_str(), password.c_str());
    updateWifiStatus(WIFI_STATUS::CONNECTING, true);
    updateWifiStatus(WIFI_STATUS::SCANNING, false);
    sendWifiStatus();
    Serial.print("Connecting to WiFi ..");

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);

        if (millis() - prevMillis > 20 * 1000)
        {
            Serial.println("Failed to connect");
            WiFi.disconnect(false, true);
            updateWifiStatus(WIFI_STATUS::CONNECTING, false);
            updateWifiStatus(WIFI_STATUS::CONNECTED, false);
            updateWifiStatus(WIFI_STATUS::SCANNING, true);
            updateWifiStatus(WIFI_STATUS::CONNECTION_FAILED, true);
            sendWifiStatus();
            return false;
        }
    }

    Serial.println("Connesso");
    updateWifiStatus(WIFI_STATUS::CONNECTING, false);
    updateWifiStatus(WIFI_STATUS::CONNECTED, true);
    updateWifiStatus(WIFI_STATUS::SCANNING, false);
    updateWifiStatus(WIFI_STATUS::CONNECTION_FAILED, false);
    sendWifiStatus();
    bleControllerPointer->setNetworkIP(WiFi.localIP().toString());

    // Se le credenziali non esisteva allora le salvo alltrimenti evito di riscriverle in memoria
    if (credentialExist == false)
    {
        saveCredential();
    }
    return true;
}

/**
 * @brief Effettua la scansione wifi delle reti nelle vicinanze
 *
 * @return String La stringa contenente tutte le reti individuate formattata come <nome rete>;<potenza>;
 */
String ConnectionController::scanNetwork()
{
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        Serial.println(F("no networks found"));
        return "";
    }

    Serial.print(n);
    Serial.println(F(" networks found"));
    String data = "";
    for (int i = 0; i < n; ++i)
    {
        data += WiFi.SSID(i);
        data += ";";
        data += String(WiFi.RSSI(i));
        data += ";";
        delay(10);
    }

    Serial.println(data);

    return data;
}

void ConnectionController::hasInternet(bool hasInternet)
{
    updateWifiStatus(WIFI_STATUS::HAS_INTERNET, hasInternet);
    sendWifiStatus();
}

bool ConnectionController::isConnected()
{
    return wifiConnectionStatus & (1 << WIFI_STATUS::CONNECTED);
}
bool ConnectionController::isConnecting()
{
    return wifiConnectionStatus & (1 << WIFI_STATUS::CONNECTING);
}
bool ConnectionController::isScanning()
{
    return wifiConnectionStatus & (1 << WIFI_STATUS::SCANNING);
}
bool ConnectionController::hasInternet()
{
    return wifiConnectionStatus & (1 << WIFI_STATUS::HAS_INTERNET);
}