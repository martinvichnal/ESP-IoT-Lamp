#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
// https://community.platformio.org/t/solved-asyncelegantota-collect2-exe-error-ld-returned-1-exit-status/28553
#include <AsyncElegantOTA.h>
#include <Arduino_JSON.h>
#include "FileSystem.h"

void WiFiInit();
void handleRoot(AsyncWebServerRequest *request);
void handleToggleLed(AsyncWebServerRequest *request);
void handleSetRGB(AsyncWebServerRequest *request);
void handleBtnState(AsyncWebServerRequest *request);

// Replace with your network credentials
const char *ssid = "UPC0130180";
const char *password = "x8wu4ztTwepF";

// Create an instance of the web server
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");

// Define pins for the LEDs
const int ledPin = 2;
const int LED1_PIN = 12;
const int LED2_PIN = 14;
const int LED3_PIN = 27;
const int RED_PIN = 26;
const int GREEN_PIN = 25;
const int BLUE_PIN = 33;

// Define variables for LED states
bool led1State = false;
bool led2State = false;
bool led3State = false;
int redValue = 0;
int greenValue = 0;
int blueValue = 0;

JSONVar readings;
JSONVar ledStates;

String getInfo()
{
  readings["info1"] = String(redValue);
  readings["info2"] = String(greenValue);
  readings["info3"] = String(blueValue);

  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  initSPIFFS();
  WiFiInit();
}
int i = 1;
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;
bool toggle = false;

void loop()
{
  if (toggle)
  {
    if ((millis() - lastTime) > timerDelay)
    {
      // Send Events to the client with the Sensor Readings Every 10 seconds
      events.send("ping", NULL, millis());
      events.send(getInfo().c_str(), "new_readings", millis());
      lastTime = millis();
    }
  }
  if (i > 2400)
  {
    i = 1;
  }
  else
  {
    redValue = i;
    greenValue = i * 2;
    blueValue = i * i;
  }
  i++;
}

void WiFiInit()
{
  // Connecting to WiFi and setting local ip
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  //---------------------------------------------------------------------------
  // Setting up the sites:

  // Setting main Index page from SPIFFS
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });
  server.serveStatic("/", SPIFFS, "/");

  // Getting RGB values from client:
  server.on("/setRGB", HTTP_GET, handleSetRGB);

  // Getting button states values from client:
  server.on("/setBtn", HTTP_GET, handleBtnState);

  // Setting the led state:
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Setting the led HIGH
    digitalWrite(ledPin, HIGH);
    Serial.println("Turning ON the leds");
    request->send(SPIFFS, "/index.html", "text/html"); });
  // Setting the led state:
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    // Setting the led LOW
    digitalWrite(ledPin, LOW);
    Serial.println("Turning Off the leds");
    request->send(SPIFFS, "/index.html", "text/html"); });

  // Setting site for JSON Testing page from SPIFFS
  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/JSONTest.html", "text/html"); });
  server.serveStatic("/", SPIFFS, "/");

  // Request for JSON
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    String json = getInfo();
    request->send(200, "application/json", json);
    json = String(); });

  events.onConnect([](AsyncEventSourceClient *client)
                   {
    if(client->lastId())
    {
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000); });
  server.addHandler(&events);

  // Starting OTA server for distant firmware and SPIFFS update
  AsyncElegantOTA.begin(&server);
  server.begin();
}

// Handler function for setting the RGB color
void handleSetRGB(AsyncWebServerRequest *request)
{
  String redStr = request->getParam("red")->value();
  String greenStr = request->getParam("green")->value();
  String blueStr = request->getParam("blue")->value();
  redValue = redStr.toInt();
  greenValue = greenStr.toInt();
  blueValue = blueStr.toInt();
  analogWrite(RED_PIN, redValue);
  analogWrite(GREEN_PIN, greenValue);
  analogWrite(BLUE_PIN, blueValue);
  Serial.print("Set RGB: (");
  Serial.print(redValue);
  Serial.print(", ");
  Serial.print(greenValue);
  Serial.print(", ");
  Serial.print(blueValue);
  Serial.println(")");
  request->send(200);
}

void handleBtnState(AsyncWebServerRequest *request)
{
  String btnStr = request->getParam("state")->value();
  int btnState = btnStr.toInt();

  if (btnState == 2)
  {
    toggle = true;
  }
  else if (btnState == 3)
  {
    toggle = false;
  }

  Serial.println(btnState);
  request->send(200);
}