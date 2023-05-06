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

// Replace with your network credentials
const char *ssid = "--";
const char *password = "--";

// Create an instance of the web server
AsyncWebServer server(80);
// Create an Event Source on /events
AsyncEventSource events("/events");

// Define pins for the LEDs
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

void loop()
{
  if ((millis() - lastTime) > timerDelay)
  {
    // Send Events to the client with the Sensor Readings Every 10 seconds
    events.send("ping", NULL, millis());
    events.send(getInfo().c_str(), "new_readings", millis());
    lastTime = millis();
  }
  if (i > 24)
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
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  // server.on("/", HTTP_GET, handleRoot);
  // server.on("/toggleLed", HTTP_GET, handleToggleLed);
  // server.on("/setRGB", HTTP_GET, handleSetRGB);

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/JSONTest.html", "text/html"); });
  server.serveStatic("/", SPIFFS, "/");

  // Request for the latest sensor readings
  server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
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

  AsyncElegantOTA.begin(&server); // Starting OTA server for distant firmware updates

  server.begin();
}

// // Handler function for the root URL
// void handleRoot(AsyncWebServerRequest *request)
// {
//   Serial.println("Received request for /");
//   String html = "";
//   html += "<html><head><title>LED Control</title></head><body>";
//   html += "<h1>LED Control</h1>";
//   html += "<p><label>LED 1:</label><input type='checkbox' onchange='toggleLed(1)'";
//   if (led1State)
//   {
//     html += " checked";
//   }
//   html += "></p>";
//   html += "<p><label>LED 2:</label><input type='checkbox' onchange='toggleLed(2)'";
//   if (led2State)
//   {
//     html += " checked";
//   }
//   html += "></p>";
//   html += "<p><label>LED 3:</label><input type='checkbox' onchange='toggleLed(3)'";
//   if (led3State)
//   {
//     html += " checked";
//   }
//   html += "></p>";
//   html += "<p><label>Red:</label><input type='range' min='0' max='255' onchange='setRGB()' id='red' value='" + String(redValue) + "'></p>";
//   html += "<p><label>Green:</label><input type='range' min='0' max='255' onchange='setRGB()' id='green' value='" + String(greenValue) + "'></p>";
//   html += "<p><label>Blue:</label><input type='range' min='0' max='255' onchange='setRGB()' id='blue' value='" + String(blueValue) + "'></p>";
//   html += "<script>";
//   html += "function toggleLed(led) {";
//   html += "var xhttp = new XMLHttpRequest();";
//   html += "xhttp.open('GET', '/toggleLed?led=' + led, true);";
//   html += "xhttp.send();";
//   html += "}";
//   html += "function setRGB() {";
//   html += "var redValue = document.getElementById('red').value;";
//   html += "var greenValue = document.getElementById('green').value;";
//   html += "var blueValue = document.getElementById('blue').value;";
//   html += "var xhttp = new XMLHttpRequest();";
//   html += "xhttp.open('GET', '/setRGB?red=' + redValue + '&green=' + greenValue + '&blue=' + blueValue, true);";
//   html += "xhttp.send();";
//   html += "}";
//   html += "</script>";
//   html += "</body></html>";
//   request->send(200, "text/html", html);
// }

// // Handler function for the toggle LED URLs
// void handleToggleLed(AsyncWebServerRequest *request)
// {
//   String ledStr = request->getParam("led")->value();
//   int led = ledStr.toInt();
//   Serial.print("Toggle LED ");
//   Serial.println(led);
//   switch (led)
//   {
//   case 1:
//     led1State = !led1State;
//     digitalWrite(LED1_PIN, led1State ? HIGH : LOW);
//     break;
//   case 2:
//     led2State = !led2State;
//     digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
//     break;
//   case 3:
//     led3State = !led3State;
//     digitalWrite(LED3_PIN, led3State ? HIGH : LOW);
//     break;
//   default:
//     break;
//   }
//   request->send(200);
// }

// // Handler function for setting the RGB color
// void handleSetRGB(AsyncWebServerRequest *request)
// {
//   String redStr = request->getParam("red")->value();
//   String greenStr = request->getParam("green")->value();
//   String blueStr = request->getParam("blue")->value();
//   redValue = redStr.toInt();
//   greenValue = greenStr.toInt();
//   blueValue = blueStr.toInt();
//   analogWrite(RED_PIN, redValue);
//   analogWrite(GREEN_PIN, greenValue);
//   analogWrite(BLUE_PIN, blueValue);
//   Serial.print("Set RGB: (");
//   Serial.print(redValue);
//   Serial.print(", ");
//   Serial.print(greenValue);
//   Serial.print(", ");
//   Serial.print(blueValue);
//   Serial.println(")");
//   request->send(200);
// }