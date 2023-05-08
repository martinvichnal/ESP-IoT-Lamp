// #include <WiFi.h>
// #include <WebServer.h>

// WebServer server(80);

// const char* ssid = "ESP32_AP";
// const char* password = "12345678";
// const char* apIP = "192.168.4.1";

// const char* wifiSSID;
// const char* wifiPassword;

// bool hasCredentials = false;

// void handleRoot() {
//   if (hasCredentials) {
//     server.send(200, "text/html", "<h1>Connected to WiFi</h1>");
//   } else {
//     String html = "<form method='post' action='/save'><label for='ssid'>SSID:</label><input type='text' id='ssid' name='ssid'><br><label for='password'>Password:</label><input type='password' id='password' name='password'><br><input type='submit' value='Submit'></form>";
//     server.send(200, "text/html", html);
//   }
// }

// void handleSave() {
//   wifiSSID = server.arg("ssid").c_str();
//   wifiPassword = server.arg("password").c_str();
//   hasCredentials = true;
//   server.send(200, "text/html", "<h1>WiFi credentials saved. Device will restart in 5 seconds.</h1>");
//   delay(5000);
//   ESP.restart();
// }

// void setup() {
//   Serial.begin(115200);
  
//   // Start AP mode
//   WiFi.softAP(ssid, password);
//   IPAddress myIP = WiFi.softAPIP();
//   Serial.print("AP mode IP address: ");
//   Serial.println(myIP);
  
//   // Set up captive portal server
//   server.on("/", handleRoot);
//   server.on("/save", handleSave);
//   server.begin();
// }

// void loop() {
//   server.handleClient();
  
//   // If we have WiFi credentials, connect to WiFi and exit
//   if (hasCredentials) {
//     Serial.println("Connecting to WiFi...");
//     WiFi.begin(wifiSSID, wifiPassword);
//     while (WiFi.status() != WL_CONNECTED) {
//       delay(1000);
//       Serial.println("Connecting to WiFi...");
//     }
//     Serial.println("Connected to WiFi.");
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
//     delay(1000);
//     ESP.restart();
//   }
// }





//=========================================================================================================================================


// #include <DNSServer.h>
// #include <WiFi.h>
// #include <AsyncTCP.h>
// #include "ESPAsyncWebServer.h"

// DNSServer dnsServer;
// AsyncWebServer server(80);

// class CaptiveRequestHandler : public AsyncWebHandler {
// public:
//   CaptiveRequestHandler() {}
//   virtual ~CaptiveRequestHandler() {}

//   bool canHandle(AsyncWebServerRequest *request){
//     //request->addInterestingHeader("ANY");
//     return true;
//   }

//   void handleRequest(AsyncWebServerRequest *request) {
//     AsyncResponseStream *response = request->beginResponseStream("text/html");
//     response->print("<!DOCTYPE html><html><head><title>Captive Portal</title></head><body>");
//     response->print("<p>This is out captive portal front page.</p>");
//     response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
//     response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());
//     response->print("</body></html>");
//     request->send(response);
//   }
// };


// void setup(){
//   //your other setup stuff...
//   WiFi.softAP("esp-captive");
//   dnsServer.start(53, "*", WiFi.softAPIP());
//   server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
//   //more handlers...
//   server.begin();
// }

// void loop(){
//   dnsServer.processNextRequest();
// }