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
#include <FastLED.h>

//==========================================================================================//
//*******************************//
// initialize fucntions
//*******************************//
void initWiFi();
void initWebServer();
void initWebSocket();
void initFastLed();

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
// Global JSON Variables
//*******************************//
bool state = false;      // 0 or 1
int mode = 0;            // 0..2
int brightnessValue = 0; // 0..255
int speedValue = 40;     // 0..100
int redValue = 0;        // 0..255
int greenValue = 0;      // 0..255
int blueValue = 0;       // 0..255

//*******************************//
// FastLED and it's variables
//*******************************//
#define LED_TYPE WS2812B         // Type of the led
#define BRIGHTNESS 255           // Starting brightness
#define COLOR_ORDER GRB          // Color Ordering
#define LED_PIN 5                // Led DATA output
const uint8_t kMatrixWidth = 10; // Width of the led arrays
const uint8_t kMatrixHeight = 4; // Height of the led arrays
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)

CRGB leds[kMatrixWidth * kMatrixHeight]; // Main led[] array
CRGB source1[kMatrixWidth * kMatrixHeight];
CRGB source2[kMatrixWidth * kMatrixHeight]; // leds[] array, later will be used for transition between modes

void changeModeWithBlend(CRGB inScene1[kMatrixWidth * kMatrixHeight], CRGB inScene2[kMatrixWidth * kMatrixHeight], bool modeChanged, int delayTime);
void runPattern(uint8_t modeActive, CRGB *LEDArray);
void CustomColor(CRGB *source);
void ColorsFade(CRGB *source);
void Rainbow(CRGB *source);
void nonBlockingDelay(unsigned long duration);

int RainbowDelayTime = 40; // Rainbow() delay time
uint8_t hue[NUM_LEDS];     // Hue value for Rainbow() effect

int ColorsFadeDelayTime = 40;     // ColorsFade() delay time
bool changeGreenAndRed = true;    // incrementing green, decrementing red
bool changeBlueAndGreeen = false; // incrementing blue, decrementing green
bool changeRedAndBlue = false;    // incrementing red, decrementing blue

int red = 0;
int green = 0;
int blue = 0;

int blendAmount = 0;
bool ModeChanged = false; // This is used for blend effect
int actualMode = 0;
int prevMode = 0;
int newMode = 0;

unsigned long previousMillis = 0;
unsigned long interval = 200;

//******************************************************************************************//
//==========================================================================================//

//==========================================================================================//
//    _____ ______________  ______              - Initializing everything
//   / ___// ____/_  __/ / / / __ \
//   \__ \/ __/   / / / / / / /_/ /
//  ___/ / /___  / / / /_/ / ____/
// /____/_____/ /_/  \____/_/
//==========================================================================================//
void setup()
{
  Serial.begin(115200);
  //    *!! when core 0 is running empty it crashes every time...
  // initCores();
  initSPIFFS(); // Initializing SPIFFS
  initWiFi();   // Initializing WiF
  initWebServer();
  initWebSocket();
  initFastLed();
}
//******************************************************************************************//
//==========================================================================================//

//==========================================================================================//
//     __    ____    ____     ____          - Loop used for WiFi things
//    / /   / __ \  / __ \   / __ \           Using Core 1
//   / /   / / / / / / / /  / /_/ /
//  / /___/ /_/ / / /_/ /  / ____/
// /_____/\____/  \____/  /_/
//==========================================================================================//
void loop()
{
  ws.cleanupClients();

  if (state)
  {
    FastLED.setBrightness(brightnessValue);
    RainbowDelayTime = speedValue;
    ColorsFadeDelayTime = speedValue;

    // If the mode has changed then start running the new mode in source2 then blend the two together
    // If the blend is done (blendAmount = 255) then change the actual mode to the next one and reset everything
    // scene1 is always the current mode effects
    // scene2 is always the new mode effects
    if (ModeChanged == true)
    {
      // Run the new mode in source2
      runPattern(actualMode, source1);
      runPattern(newMode, source2);

      // Increment blend amount until 255 (which is the max and the led is driven with source2)
      if (blendAmount > 255)
      {
        blendAmount = 255;
      }
      else if (blendAmount == 255)
      {
        // Giving the actial mode the new mode
        actualMode = newMode;
        ModeChanged = false;
      }

      blend(source1, source2, leds, NUM_LEDS, blendAmount);

      blendAmount++;

      if (ModeChanged == false)
      {
        blendAmount = 0;
      }

      Serial.print("actualMode: ");
      Serial.println(actualMode);
      Serial.print("blendAmount: ");
      Serial.println(blendAmount);
      Serial.print("ModeChanged: ");
      Serial.println(ModeChanged);
      Serial.println();
    }
    else
    {
      // Running the actualMode in leds
      runPattern(actualMode, leds);
    }
  }
  else
  {
    FastLED.setBrightness(0);
  }
  // delay(50);
  FastLED.show();
}
//******************************************************************************************//
//==========================================================================================//

//==========================================================================================//
//    __________  ____  ______       __    ____  ____  ____       - Used for driving the
//   / ____/ __ \/ __ \/ ____/      / /   / __ \/ __ \/ __ \        LEDS with FastLED
//  / /   / / / / /_/ / __/        / /   / / / / / / / /_/ /        Library
// / /___/ /_/ / _, _/ /___       / /___/ /_/ / /_/ / ____/         This is Core 0
// \____/\____/_/ |_/_____/      /_____/\____/\____/_/              (Arduino using core 1)
//==========================================================================================//
void core0Func(void *)
{
  //  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  THIS CANT BE EMPTY BECAUSE IT CRASHES AND REBOOTS IF EMPTY   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // Inf. loop for core fucntions:
  while (true)
  {
    notifyClients();
    delay(50);
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
  server.on("/wifimanager", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/wifimanager.html", "text/html"); });
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
    if (prevMode != mode)
    {
      prevMode = mode;
      newMode = mode;
      ModeChanged = true;
      Serial.println("MODE CHANGED");
    }

    brightnessValue = json["brightnessValue"];
    speedValue = json["speedValue"];
    redValue = json["redValue"];
    greenValue = json["greenValue"];
    blueValue = json["blueValue"];

    // Serial.print("state:    ");
    // Serial.println(state);
    // Serial.print("mode:   ");
    // Serial.println(mode);
    // Serial.print("brightnessValue:    ");
    // Serial.println(brightnessValue);
    // Serial.print("speedValue:   ");
    // Serial.println(speedValue);
    // Serial.print("redValue:   ");
    // Serial.println(redValue);
    // Serial.print("greenValue:   ");
    // Serial.println(greenValue);
    // Serial.print("blueValue:    ");
    // Serial.println(blueValue);
    // Serial.println();
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

void initFastLed()
{
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.clear();
  FastLED.show();
}

void runPattern(uint8_t modeActive, CRGB *LEDArray)
{
  switch (modeActive)
  {
  case 0:
  {
    if (millis() - previousMillis >= RainbowDelayTime)
    {
      previousMillis = millis();
      Rainbow(LEDArray);
      // Serial.println("Rainbow()");
    }
    // fill_solid(LEDArray, NUM_LEDS, CRGB(255, 0, 0));
    break;
  }

  case 1:
  {
    if (millis() - previousMillis >= ColorsFadeDelayTime)
    {
      previousMillis = millis();
      ColorsFade(LEDArray);
      // Serial.println("ColorsFade()");
    }
    // fill_solid(LEDArray, NUM_LEDS, CRGB(0, 255, 0));
    break;
  }

  case 2:
  {
    CustomColor(LEDArray);
    // Serial.println("CustomColor()");
    // fill_solid(LEDArray, NUM_LEDS, CRGB(0, 0, 255));
    break;
  }
  }
}

// Setting the leds to a custom hue value.
void CustomColor(CRGB *source)
{
  fill_solid(source, NUM_LEDS, CRGB(redValue, greenValue, blueValue));
}

// Fading the colors.
void ColorsFade(CRGB *source)
{
  /////////////////////////////green & red////////////////////////////////
  if (changeGreenAndRed)
  {
    fill_solid(source, NUM_LEDS, CRGB(red, green, blue));
    green++;
    red--;

    if ((red <= 0) || (green >= 255))
    {
      green = 255;
      red = 0;
      changeGreenAndRed = false;
      changeBlueAndGreeen = true;
      changeRedAndBlue = false;
    }
  }

  ///////////////////////////////blue & green//////////////////////////////
  if (changeBlueAndGreeen)
  {
    fill_solid(source, NUM_LEDS, CRGB(red, green, blue));
    blue++;
    green--;

    if ((green <= 0) || (blue >= 255))
    {
      blue = 255;
      green = 0;
      changeGreenAndRed = false;
      changeBlueAndGreeen = false;
      changeRedAndBlue = true;
    }
  }

  ////////////////////////////////red & blue/////////////////////////////
  if (changeRedAndBlue)
  {
    fill_solid(source, NUM_LEDS, CRGB(red, green, blue));
    red++;
    blue--;

    if ((blue <= 0) || (red >= 255))
    {
      red = 255;
      blue = 0;
      changeGreenAndRed = true;
      changeBlueAndGreeen = false;
      changeRedAndBlue = false;
    }
  }
}
// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// Rainbow effect
void Rainbow(CRGB *source)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    source[i] = CHSV(hue[i]++, 255, 255);
  }
}