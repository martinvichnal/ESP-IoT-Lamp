#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
//https://community.platformio.org/t/solved-asyncelegantota-collect2-exe-error-ld-returned-1-exit-status/28553


// Replace with your network credentials
const char *ssid = "//";
const char *password = "//";

// Create an instance of the web server
AsyncWebServer server(80);

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

// Handler function for the root URL
void handleRoot(AsyncWebServerRequest *request)
{
  Serial.println("Received request for /");
  String html = "";
  html += "<html><head><title>LED Control</title></head><body>";
  html += "<h1>LED Control</h1>";
  html += "<p><label>LED 1:</label><input type='checkbox' onchange='toggleLed(1)'";
  if (led1State)
  {
    html += " checked";
  }
  html += "></p>";
  html += "<p><label>LED 2:</label><input type='checkbox' onchange='toggleLed(2)'";
  if (led2State)
  {
    html += " checked";
  }
  html += "></p>";
  html += "<p><label>LED 3:</label><input type='checkbox' onchange='toggleLed(3)'";
  if (led3State)
  {
    html += " checked";
  }
  html += "></p>";
  html += "<p><label>Red:</label><input type='range' min='0' max='255' onchange='setRGB()' id='red' value='" + String(redValue) + "'></p>";
  html += "<p><label>Green:</label><input type='range' min='0' max='255' onchange='setRGB()' id='green' value='" + String(greenValue) + "'></p>";
  html += "<p><label>Blue:</label><input type='range' min='0' max='255' onchange='setRGB()' id='blue' value='" + String(blueValue) + "'></p>";
  html += "<script>";
  html += "function toggleLed(led) {";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.open('GET', '/toggleLed?led=' + led, true);";
  html += "xhttp.send();";
  html += "}";
  html += "function setRGB() {";
  html += "var redValue = document.getElementById('red').value;";
  html += "var greenValue = document.getElementById('green').value;";
  html += "var blueValue = document.getElementById('blue').value;";
  html += "var xhttp = new XMLHttpRequest();";
  html += "xhttp.open('GET', '/setRGB?red=' + redValue + '&green=' + greenValue + '&blue=' + blueValue, true);";
  html += "xhttp.send();";
  html += "}";
  html += "</script>";
  html += "</body></html>";
  request->send(200, "text/html", html);
}

// Handler function for the toggle LED URLs
void handleToggleLed(AsyncWebServerRequest *request)
{
  String ledStr = request->getParam("led")->value();
  int led = ledStr.toInt();
  Serial.print("Toggle LED ");
  Serial.println(led);
  switch (led)
  {
  case 1:
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State ? HIGH : LOW);
    break;
  case 2:
    led2State = !led2State;
    digitalWrite(LED2_PIN, led2State ? HIGH : LOW);
    break;
  case 3:
    led3State = !led3State;
    digitalWrite(LED3_PIN, led3State ? HIGH : LOW);
    break;
  default:
    break;
  }
  request->send(200);
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

void setup()
{
  Serial.begin(115200);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/toggleLed", HTTP_GET, handleToggleLed);
  server.on("/setRGB", HTTP_GET, handleSetRGB);
  AsyncElegantOTA.begin(&server);
  server.begin();
}

void loop()
{
  // Nothing to do here
}