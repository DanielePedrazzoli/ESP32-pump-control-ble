#ifndef API_CONTROLLER
#define API_CONTROLLER

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <esp_http_client.h>

class ApiController
{
private:
public:
  ApiController();

  void init();
  void requestWeatherData(JsonDocument *doc);
};

#endif