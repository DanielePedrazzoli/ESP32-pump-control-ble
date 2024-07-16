#include "API_Controller.h"
#include "esp_http_client.h"

ApiController::ApiController() {}

void ApiController::init()
{
}

void ApiController::requestWeatherData(JsonDocument *doc)
{

    HTTPClient client;
    client.begin(F("https://api.open-meteo.com/v1/forecast?latitude=44.8396&longitude=10.7543&current=rain&hourly=temperature_2m,precipitation_probability,precipitation,weather_code&timezone=auto&forecast_days=1"));

    int httpCode = client.GET();

    if (httpCode <= 0)
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", client.errorToString(httpCode).c_str());
        client.end();
    }

    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = client.getString();
        DeserializationError error = deserializeJson(*doc, payload);
        if (error)
        {
            Serial.print("deserializeJson() failed: ");
            Serial.println(error.c_str());
        }
    }

    client.end();
}
