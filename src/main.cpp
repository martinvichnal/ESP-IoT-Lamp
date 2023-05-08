#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// https://community.platformio.org/t/solved-asyncelegantota-collect2-exe-error-ld-returned-1-exit-status/28553
#include <AsyncElegantOTA.h>
// #include <Arduino_JSON.h>
// https://github.com/arduino-libraries/Arduino_JSON
#include <ArduinoJson.h>
#include "FileSystem.h"

//==========================================================================================//
//*******************************//
// initialize fucntions
//*******************************//
void initWiFi();
void initWebServer();
void initWebSocket();

//*******************************//
// web page fucntions
//*******************************//
void handleRoot(AsyncWebServerRequest *request);
void handleToggleLed(AsyncWebServerRequest *request);
void handleSetRGB(AsyncWebServerRequest *request);
void handleBtnState(AsyncWebServerRequest *request);

//*******************************//
// WebSocket fucntions
//*******************************//
void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len);
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void notifyClients();

//*******************************//
// Multi Core fucntions
//*******************************//
TaskHandle_t Core0; // will be used for wifi
void initCores();
void core0Func(void *);


//*******************************//
// WiFi
//*******************************//
#define HTTP_PORT 80
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");

// Replace with your network credentials
const char *ssid = "";
const char *password = "";

//*******************************//
// Variables
//*******************************//
bool state = false;
int mode = 0;
int brightnessValue = 0;
int speedValue = 0;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;
//******************************************************************************************//
//==========================================================================================//


//==========================================================================================//
//    _____ ______________  ______ 
//   / ___// ____/_  __/ / / / __ \
//   \__ \/ __/   / / / / / / /_/ /
//  ___/ / /___  / / / /_/ / ____/ 
// /____/_____/ /_/  \____/_/      
//==========================================================================================//
void setup()
{
  Serial.begin(115200);
  //    *!! when core 0 is running empty it crashes every time...
  initCores();
}
//******************************************************************************************//
//==========================================================================================//

//==========================================================================================//
//     __    ____    ____     ____
//    / /   / __ \  / __ \   / __ \
//   / /   / / / / / / / /  / /_/ /
//  / /___/ /_/ / / /_/ /  / ____/
// /_____/\____/  \____/  /_/
//==========================================================================================//
void loop()
{

}
//******************************************************************************************//
//==========================================================================================//



//==========================================================================================//
//    __________  ____  ______       __    ____  ____  ____ 
//   / ____/ __ \/ __ \/ ____/      / /   / __ \/ __ \/ __ \
//  / /   / / / / /_/ / __/        / /   / / / / / / / /_/ /
// / /___/ /_/ / _, _/ /___       / /___/ /_/ / /_/ / ____/ 
// \____/\____/_/ |_/_____/      /_____/\____/\____/_/      
//==========================================================================================//
int dummy = 0;
void core0Func(void *)
{
  initSPIFFS(); // Initializing SPIFFS
  initWiFi();   // Initializing WiF
  initWebServer();
  initWebSocket();

  // Inf. loop for core fucntions:
  while (true)
  {
    ws.cleanupClients();
    // delay(50);
    // mode = dummy;
    // brightnessValue = dummy + dummy;
    // speedValue = dummy * 2;
    // dummy++;
    delay(1000);
    // notifyClients();

    // Serial.print("state:    "); Serial.println(state);
    // Serial.print("mode:   "); Serial.println(mode);
    // Serial.print("brightnessValue:    "); Serial.println(brightnessValue);
    // Serial.print("speedValue:   "); Serial.println(speedValue);
    // Serial.print("redValue:   "); Serial.println(redValue);
    // Serial.print("greenValue:   "); Serial.println(greenValue);
    // Serial.print("blueValue:    "); Serial.println(blueValue);
    // Serial.println();
  }
}
//******************************************************************************************//
//==========================================================================================//


//==========================================================================================//
//     ________  ___   ______________________  _   _______
//    / ____/ / / / | / / ____/_  __/  _/ __ \/ | / / ___/
//   / /_  / / / /  |/ / /     / /  / // / / /  |/ /\__ \ 
//  / __/ / /_/ / /|  / /___  / / _/ // /_/ / /|  /___/ / 
// /_/    \____/_/ |_/\____/ /_/ /___/\____/_/ |_//____/  
//==========================================================================================//
void initWiFi()
{
  // Connecting to WiFi and setting local ip
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("WiFi connected. IP address: ");
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

void initWebServer()
{
  //  Setting up sites:
  //    * "/" root page - index.html (from SPIFFS)
  //    * "/setRGB" requesting RGB walues from sliders - connected with handleSetRGB()
  //    * "/setBtn" requesting Button state values (0, 1, 2, 3) - connected with handleBtnState()

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.on("/setRGB", HTTP_GET, handleSetRGB);
  server.on("/setBtn", HTTP_GET, handleBtnState);

  server.serveStatic("/", SPIFFS, "/");
  AsyncElegantOTA.begin(&server); // Starting OTA server for distant firmware and SPIFFS update
  server.begin();
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{ //

  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    const uint8_t size = JSON_OBJECT_SIZE(7);
    StaticJsonDocument<size> json;
    DeserializationError err = deserializeJson(json, data);
    // ERROR
    if (err)
    {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }

    state = json["state"];
    mode = json["mode"];
    brightnessValue = json["brightnessValue"];
    speedValue = json["speedValue"];
    redValue = json["redValue"];
    greenValue = json["greenValue"];
    blueValue = json["blueValue"];

    // int _redValue = json["redValue"];
    Serial.println(redValue);

    // const char *_state = json["state"];
    // Serial.println(_state);
    // if (strcmp(_state, "1") == 0)
    // {
    //   state = true;
    //   Serial.println(state);
    // }
    // if (strcmp(_state, "0") == 0)
    // {
    //   state = false;
    //   Serial.println(state);
    // }
  }
  notifyClients();
}

void notifyClients()
{
  const uint8_t size = JSON_OBJECT_SIZE(7);
  StaticJsonDocument<size> json;
  json["state"] = state;
  json["mode"] = mode;
  json["brightnessValue"] = brightnessValue;
  json["speedValue"] = speedValue;
  json["redValue"] = redValue;
  json["greenValue"] = greenValue;
  json["blueValue"] = blueValue;

  // char data[17];
  char data[size];
  size_t len = serializeJson(json, data);
  ws.textAll(data, len);
}

// Handler function for setting the RGB color
void handleSetRGB(AsyncWebServerRequest *request)
{
  // String redStr = request->getParam("red")->value();
  // String greenStr = request->getParam("green")->value();
  // String blueStr = request->getParam("blue")->value();
  // redValue = redStr.toInt();
  // greenValue = greenStr.toInt();
  // blueValue = blueStr.toInt();
  // analogWrite(RED_PIN, redValue);
  // analogWrite(GREEN_PIN, greenValue);
  // analogWrite(BLUE_PIN, blueValue);
  // Serial.print("Set RGB: (");
  // Serial.print(redValue);
  // Serial.print(", ");
  // Serial.print(greenValue);
  // Serial.print(", ");
  // Serial.print(blueValue);
  // Serial.println(")");
  // request->send(200);
}

void handleBtnState(AsyncWebServerRequest *request)
{
  // String btnStr = request->getParam("state")->value();
  // int btnState = btnStr.toInt();

  // Serial.println(btnState);
  // request->send(200);
}

void initCores()
{
  // Initializing Core 0 for WIFI tasks
  xTaskCreatePinnedToCore(
      core0Func, /* Task function. */
      "Core0",   /* name of task. */
      10000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Core0,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  delay(500);
}