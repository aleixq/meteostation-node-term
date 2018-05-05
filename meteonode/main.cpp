/**
   Meteostation-node

*/


#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include "wifi-credentials.h"


#include <SoftwareSerial.h>
// 2 software serial ; el de usb per debug i el SOFTWARE SERIAL per comunicar-se amb arduino
SoftwareSerial ARDUINO_SERIAL(5, 4, false, 256); // GPIO ONES, EQUIVAL a D5 i D6

#define USE_SERIAL Serial

ESP8266WebServer server(80);
String cached_indata = "";

HTTPClient http;

const int led = 13;

void handleRoot();
void handleMeteoData();
void handleNotFound();
void handleMeteoData();
int readline(int readch, char *buffer, int len);
boolean WiFiReturns();


void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}


void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  USE_SERIAL.begin(115200);
  USE_SERIAL.setDebugOutput(true);
  USE_SERIAL.println();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  for (uint8_t t = 5; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }


  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("meteoNode")) {
    Serial.println("MDNS responder started");
  }

  // Add service to MDNS-SD
  MDNS.addService("meteo", "tcp", 80);


  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.on("/getMeteoData", handleMeteoData);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");

  ARDUINO_SERIAL.begin(9600);
}

void loop() {
    if (!WiFiReturns()) {
      WiFi.reconnect(); //Connection OFF
    } else {
        //connection ON
      server.handleClient();
      while (ARDUINO_SERIAL.available()) {
          String inData = ARDUINO_SERIAL.readStringUntil('\n');
          Serial.println("Feeding cache: " + inData);
          if (inData.startsWith("[")){
            cached_indata = String(inData);
          }
      }
      return;
    }
}

void handleMeteoData() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", String(cached_indata));
    digitalWrite(led, 0);

}


int readline(int readch, char *buffer, int len)
{
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': // Ignore new-lines
        break;
      case '\r': // Return on CR
        rpos = pos;
        pos = 0;  // Reset position index ready for next time
        return rpos;
      default:
        if (pos < len-1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
  }
  // No end of line has been found, so return -1.
  return -1;
}

boolean WiFiReturns() {
if (WiFi.localIP() == IPAddress(0, 0, 0, 0)) return 0;
  switch (WiFi.status()) {
    case WL_NO_SHIELD: return 0;
    case WL_IDLE_STATUS: return 0;
    case WL_NO_SSID_AVAIL: return 0;
    case WL_SCAN_COMPLETED: return 1;
    case WL_CONNECTED: return 1;
    case WL_CONNECT_FAILED: return 0;
    case WL_CONNECTION_LOST: return 0;
    case WL_DISCONNECTED: return 0;
    default: return 0;
  }
}
