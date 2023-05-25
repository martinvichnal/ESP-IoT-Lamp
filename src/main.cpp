#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// https://community.platformio.org/t/solved-asyncelegantota-collect2-exe-error-ld-returned-1-exit-status/28553
#include <AsyncElegantOTA.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "FileSystem.h"
#include "Cipher.h" // For encryption

//==========================================================================================//
//*******************************//
// initialize fucntions
//*******************************//
void initWiFi();
void initWebServer();
void initWebSocket();
void initWiFiManager();
void initFastLed();

// Cipher *cipher = new Cipher();

//*******************************//
// web page fucntions
//*******************************//
void handleRoot(AsyncWebServerRequest *request);
void handleToggleLed(AsyncWebServerRequest *request);
void handleSetRGB(AsyncWebServerRequest *request);
void handleBtnState(AsyncWebServerRequest *request);

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/wifimanager.html", "text/html", false);
  }
};

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
// TaskHandle_t Task1, Task2;
void initCores();
void core0Func(void *);

//*******************************//
// WiFi
//*******************************//
#define HTTP_PORT 80
AsyncWebServer server(HTTP_PORT);
AsyncWebSocket ws("/ws");
DNSServer dnsServer;

// Replace with your network credentials
const char *ssid = "UPC0130180";
const char *password = "x8wu4ztTwepFl";

String get_ssid;
String get_pass;
bool ssid_received = false;
bool pass_received = false;

int tryCount = 0;     // if this is 120 (tryForMs / delayCount) it goes into AP mode
int tryForMs = 30000; // Trying to connect to the internet for 1 min (60000 ms)
int delayCount = 500;
bool connected = false;

//*******************************//
// Global JSON Variables
//*******************************//
bool state = true;         // 0 or 1
int mode = 0;              // 0..2
int brightnessValue = 100; // 0..255
int speedValue = 40;       // 0..100
int rgbValue = 255;        // 0..255
// int redValue = 255;        // 0..255
// int greenValue = 24;       // 0..255
// int blueValue = 0;         // 0..255

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
void LEDs_Darking(int brightness, int dTime);  // Darkening the leds
void LEDs_Lighting(int brightness, int dTime); // Lightning the leds

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

int prevState = 0;
bool stateChanged = false;

int prevBrightness = 0;

unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
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

  // Setup encode key
  // Setting up encrypted key
  // char *key = "oknclihdog";
  // cipher->setKey(key);

  initSPIFFS(); // Initializing SPIFFS
  initCores();
  initFastLed();

  // initWiFi();
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
  if ((state == true) && (stateChanged == false))
  {
    FastLED.setBrightness(brightnessValue);
    RainbowDelayTime = speedValue;
    ColorsFadeDelayTime = speedValue;

    if (ModeChanged != true)
    {
      // Running the actualMode in leds
      runPattern(actualMode, leds);
    }
    else
    {
      // If the mode has changed then start running the new mode in source2 then blend the two together
      // If the blend is done (blendAmount = 255) then change the actual mode to the next one and reset everything
      // scene1 is always the current mode effects
      // scene2 is always the new mode effects

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
        blendAmount = 0;
      }
      else
      {
        blendAmount++;
      }

      blend(source1, source2, leds, NUM_LEDS, blendAmount);
    }
  }
  else if ((state == false) && (stateChanged == true))
  {
    prevBrightness = FastLED.getBrightness();
    LEDs_Darking(FastLED.getBrightness(), 2);
    stateChanged = false;
  }
  else if ((state == true) && (stateChanged == true))
  {
    LEDs_Lighting(prevBrightness, 2);
    stateChanged = false;
  }

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
  initWiFi();
  while (true)
  {
    if (connected)
    {
      ws.cleanupClients();
      notifyClients();
      delay(500);
    }
    // else if (!connected)
    // {
    //   initWiFi();
    // }
    // else
    // {
    //   dnsServer.processNextRequest();
    // }

    // // If SSID and Pass received initWiFi() starts again and tries to connect to WiFi.
    // if ((name_received == true) && (pass_received == true))
    // {
    //   initWiFi();
    //   name_received = false;
    //   pass_received = false;
    // }
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

//**************************************************************//
// void initWiFi()
//  - First starts a server with given ssid and pass. If can't
//    connect starts the AP mode process (initWiFiManager())
//    If can connect starts the webserver (initWebServer())
//    with index.html and starts the websocket (initWebSocket())
void initWiFi()
{
  // First Trying to connect to the internet
  // Connecting to WiFi and setting local ip
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");

  while (WiFi.status() != WL_CONNECTED)
  {
    // Exchange for delay(delayCount)
    if (millis() - previousMillis2 >= delayCount)
    {
      previousMillis2 = millis();

      Serial.print(".");
      tryCount++;
      if ((tryCount * delayCount) >= tryForMs)
      {
        connected = false; // ESP32 cannot connect to the wifi -> Starting Wifi Manager (AP mode)
        break;
      }
      else
      {
        connected = true;
      }
    }
  }

  if (connected)
  {
    Serial.println();
    Serial.print("WiFi connected. IP address: ");
    Serial.printf(" %s\n", WiFi.localIP().toString().c_str());

    initWebServer();
    initWebSocket();
  }
  else if (!connected)
  {
    Serial.println();
    Serial.println("Cannot connect to WiFi.");
    Serial.println("Starting WiFi manager process...");

    // initWiFiManager();
  }
  
}

//**************************************************************//
// void initWiFiManager()
//  - Starts AP process and casts the wifimanager.html from
//    SPIFFS to enter a custom SSID and Passowrd
void initWiFiManager()
{
  Serial.println();
  Serial.println("Setting up AP Mode");
  WiFi.mode(WIFI_AP);
  WiFi.softAP("esp-captive");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  Serial.println("Setting up Async WebServer");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      request->send(SPIFFS, "/wifimanager.html","text/html", false); 
      Serial.println("Client Connected"); });

  server.on("/get", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String inputMessage;
      String inputParam;
  
      if (request->hasParam("ssid")) 
      {
        inputMessage = request->getParam("ssid")->value();
        inputParam = "ssid";
        get_ssid = inputMessage;
        Serial.println(inputMessage);
        ssid_received = true;
      }

      if (request->hasParam("pass")) 
      {
        inputMessage = request->getParam("pass")->value();
        inputParam = "pass";
        get_pass = inputMessage;
        Serial.println(inputMessage);
        pass_received = true;
      }
      request->send(200, "text/html", "The values entered by you have been successfully sent to the device <br><a href=\"/\">Return to Home Page</a>"); });

  Serial.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
  server.begin();
  Serial.println("All Done!");
}

//**************************************************************//
// void initWebServer()
//  - Starts webserver
//    * "/" root page - index.html (from SPIFFS)
//    * "/setRGB" requesting RGB walues from sliders - connected with handleSetRGB()
//    * "/setBtn" requesting Button state values (0, 1, 2, 3) - connected with handleBtnState()
void initWebServer()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  // server.on("/setRGB", HTTP_GET, handleSetRGB);
  // server.on("/setBtn", HTTP_GET, handleBtnState);

  server.serveStatic("/", SPIFFS, "/");
  AsyncElegantOTA.begin(&server); // Starting OTA server for distant firmware and SPIFFS update
  server.begin();
}

//**************************************************************//
// void initWebSocket()
//  - Starts websocket
void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

//**************************************************************//
// void onEvent(params)
//  - Handles messages from websocket. ie: connect, disconnect
//    err and messages. The messages then sent to
//    handleWebSocketMessage(arg, data, len);
void onEvent(AsyncWebSocket *server,
             AsyncWebSocketClient *client,
             AwsEventType type,
             void *arg,
             uint8_t *data,
             size_t len)
{
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

//**************************************************************//
// void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
//  - Handles messages from JSON and gives data to the
//    main variables.
//  - Notifying every client when getting a new message.
void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    const uint8_t size = JSON_OBJECT_SIZE(5);
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
    if (prevState != state)
    {
      prevState = state;
      stateChanged = true;
    }

    mode = json["mode"];
    if (prevMode != mode)
    {
      prevMode = mode;
      newMode = mode;
      ModeChanged = true;
    }

    brightnessValue = json["brightnessValue"];
    speedValue = json["speedValue"];
    rgbValue = json["rgbValue"];
    // redValue = json["redValue"];
    // greenValue = json["greenValue"];
    // blueValue = json["blueValue"];
  }
  notifyClients();
}

//**************************************************************//
// void notifyClients()
//  - Notifying every client with JSON
void notifyClients()
{
  const uint8_t size = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<size> json;
  json["state"] = state;
  json["mode"] = mode;
  json["brightnessValue"] = brightnessValue;
  json["speedValue"] = speedValue;
  json["rgbValue"] = rgbValue;
  // json["redValue"] = redValue;
  // json["greenValue"] = greenValue;
  // json["blueValue"] = blueValue;

  // char data[17];
  char data[size];
  size_t len = serializeJson(json, data);
  ws.textAll(data, len);
}

//**************************************************************//
// void initCores()
//  - Starts ESP32 core 0 running with core0Func() task
void initCores()
{
  // Initializing Core 0 for WiFi tasks
  xTaskCreatePinnedToCore(
      core0Func, /* Task function. */
      "Core0",   /* name of task. */
      9000,     /* Stack size of task */
      NULL,      /* parameter of the task */
      1,         /* priority of the task */
      &Core0,    /* Task handle to keep track of created task */
      0);        /* pin task to core 0 */
  // delay(500);
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
    }
    break;
  }

  case 1:
  {
    if (millis() - previousMillis >= ColorsFadeDelayTime)
    {
      previousMillis = millis();
      ColorsFade(LEDArray);
    }
    break;
  }

  case 2:
  {
    CustomColor(LEDArray);
    break;
  }
  }
}

// Setting the leds to a custom hue value.
void CustomColor(CRGB *source)
{
  // fill_solid(source, NUM_LEDS, CRGB(redValue, greenValue, blueValue));
  fill_solid(source, NUM_LEDS, CHSV(rgbValue, 255, 255));
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

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
// Darkening the leds
// ibrightness = the current brightnessValue, idTime = delay time
void LEDs_Darking(int brightness, int dTime)
{
  while (brightness > 0)
  {
    brightness--;
    if (brightness > 50)
      brightness--;
    if (brightness > 100)
      brightness--;
    if (brightness > 200)
      brightness--;
    if (brightness < 0)
      brightness = 0;
    if (brightness < 20)
      brightness = 0;
    FastLED.setBrightness(brightness);
    FastLED.show();
    delay(dTime); // (1)
  }
}

// @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@

// Lightning the leds
// ibrightness = the current brightnessValue, idTime = delay time
void LEDs_Lighting(int brightness, int dTime)
{
  int cnt = 20;
  while (cnt < brightness)
  {
    cnt++;
    if (cnt > 50)
      cnt++;
    if (cnt > 100)
      cnt++;
    if (cnt > 200)
      cnt++;
    if (cnt > brightness)
      cnt = brightness;
    FastLED.setBrightness(cnt);
    FastLED.show();
    delay(dTime); // (1)
  }
}

// // Handler function for setting the RGB color
// void handleSetRGB(AsyncWebServerRequest *request)
// {
//   // String redStr = request->getParam("red")->value();
//   // String greenStr = request->getParam("green")->value();
//   // String blueStr = request->getParam("blue")->value();
//   // redValue = redStr.toInt();
//   // greenValue = greenStr.toInt();
//   // blueValue = blueStr.toInt();
//   // analogWrite(RED_PIN, redValue);
//   // analogWrite(GREEN_PIN, greenValue);
//   // analogWrite(BLUE_PIN, blueValue);
//   // Serial.print("Set RGB: (");
//   // Serial.print(redValue);
//   // Serial.print(", ");
//   // Serial.print(greenValue);
//   // Serial.print(", ");
//   // Serial.print(blueValue);
//   // Serial.println(")");
//   // request->send(200);
// }

// void handleBtnState(AsyncWebServerRequest *request)
// {
//   // String btnStr = request->getParam("state")->value();
//   // int btnState = btnStr.toInt();

//   // Serial.println(btnState);
//   // request->send(200);
// }