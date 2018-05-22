#include <TimeLib.h>

#include <Arduino.h>
#include <SPI.h>

#include "wifi-credentials.h"
#include <Adafruit_ILI9341esp.h>
#include <Adafruit_GFX.h>
#include <XPT2046.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <WiFiUdp.h>

#include "weathertake.h"
#include "WeatherLogger.h"
#include "layout.h"
#include "Forecaster.h"
#include "wu_icons.h";



ESP8266WiFiMulti WiFiMulti;

// Modify the following two lines to match your hardware
// Also, update calibration parameters below, as necessary

// For the esp shield, these are the default.
#define TFT_DC 2
#define TFT_CS 15

// CS for SD reader
#define SD_CS 4
File f; // main history file
File fb; // secondary history file
int historyBackupFrequency= 600;
unsigned long lastHistoryBackup=0;

//char meteoNode[16] = {0};
IPAddress meteoNode ;
int meteoNodePort = 80;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
XPT2046 touch(/*cs=*/ 4, /*irq=*/ 5);

// Discover and refresh frequency in s
int NEXT_REFRESH_IN = 0; //int holding when will occur the next refresh
int REFRESH_FREQ = 5;
long lastSecond; //The seconds counter to see when a second rolls by


// # Weather VARS
float humitat = 0; // [%]
float temperaturac = 0; //  [temperature C]
float dewPoint;
float barometer_mB = 1020.0;
float pressio = 0;
float windspeedkph = 0;
int windgustdir = 0; // [0-360 using software specific time period]
float rainin = 0; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
volatile float dailyrainin = 0; // [rain inches so far today in local time]

time_t timeNow  = 1524259887;

// # Forecast Vars
const int older_sample = 12;
const int samples = 4;
WeatherTake history[samples];

const bool N_HEMISPHERE = true; //north_hemisphere
#define FORECASTARRAYSIZE 27

char* forecast[FORECASTARRAYSIZE] = {
    "Settled fine", //0
    "Bon temps",
    "Becoming fine",
    "Fine, becoming\n less settled",
    "Fine, possible\n showers", //5
    "Fairly fine,\n improving",
    "Fairly fine,\n possible showers early",
    "Fairly fine,\n showery later",
    "Showery early,\n improving",
    "Changeable, mending", //10
    "Fairly fine,\n showers ",//likely",//ly",
    "Rather unsettled\n clearing later",
    "Unsettled, probably\n improving",
    "Showery, bright\n intervals",
    "Showery, becoming\n less settled", //15
    "Changeable, some\n rain",
    "Unsettled, short\n fine intervals",
    "Unsettled, rain\n later",
    "Unsettled, some\n rain",
    "Mostly very\n unsettled", //20
    "Occasional rain,\n worsening",
    "Rain at times,\n very unsettled",
    "Rain at frequent\n intervals",
    "Rain, very\n unsettled",
    "Stormy, may\n improve", //25
    "Stormy, much rain"
};
// equivalents of Zambretti 'dial window' letters A - Z
int rise_options[FORECASTARRAYSIZE]  = {25,25,25,24,24,19,16,12,11,9,8,6,5,2,1,1,0,0,0,0,0,0} ;
int steady_options[FORECASTARRAYSIZE] = {25,25,25,25,25,25,23,23,22,18,15,13,10,4,1,1,0,0,0,0,0,0} ;
int fall_options[FORECASTARRAYSIZE] =  {25,25,25,25,25,25,25,25,23,23,21,20,17,14,7,3,1,1,1,0,0,0} ;

// # Layout Vars
int margin = 5;
int rowSize = 0;
int columnSize = 0;

// # TIME VARS


// NTP Servers:
static const char ntpServerName[] = "pool.ntp.org";
//static const char ntpServerName[] = "time.nist.gov";
//static const char ntpServerName[] = "time-a.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-b.timefreq.bldrdoc.gov";
//static const char ntpServerName[] = "time-c.timefreq.bldrdoc.gov";

const int timeZone = 2;     // Central European Time
//const int timeZone = -5;  // Eastern Standard Time (USA)
//const int timeZone = -4;  // Eastern Daylight Time (USA)
//const int timeZone = -8;  // Pacific Standard Time (USA)
//const int timeZone = -7;  // Pacific Daylight Time (USA)

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

// # Connection VARS
WiFiUDP Udp;
unsigned int localPort = 8888; // local port to listen for UDP packets
boolean WiFiReturns();

time_t getNtpTime();
void sendNTPpacket(IPAddress &address);

// # File VARS
String fileRead(String);
void fsInit(void);




void setup() {
    delay(1000);

    Serial.begin(115200);
    // Initialize Filesystem
    Serial.println("Opening SPIFFS");
    fsInit();

    SPI.setFrequency(ESP_SPI_FREQ);

    tft.begin();
    touch.begin(tft.width(), tft.height());  // Must be done before setting rotation
    tft.setRotation(3);
    rowSize = tft.height() / 5;
    columnSize = (tft.width() - margin) / 6;
    Serial.print("tftx ="); Serial.print(tft.width()); Serial.print(" tfty ="); Serial.println(tft.height());
    tft.fillScreen(ILI9341_BLACK);
    // Replace these for your screen module
    touch.setCalibration(209, 1759, 1775, 273); // it's set in portrait
    touch.setRotation(touch.ROT270);

    // WIFI PART
    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
    WiFi.mode(WIFI_STA);
    WiFiMulti.addAP(ssid, password);

    while (WiFiMulti.run() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    if (!MDNS.begin("terminalMeteo")) {
        Serial.println("Error setting up MDNS responder!");
    }
    Serial.println("Sending mDNS query");
    int n = MDNS.queryService("meteo", "tcp"); // Send out query for esp tcp services
    Serial.println("mDNS query done");
    while (n == 0) {
        Serial.println("no services found");
        n = MDNS.queryService("meteo", "tcp"); // Send out query for esp tcp services
        Serial.println("mDNS re-query done");
        delay(2000);
    }
    if (n > 0) {
        Serial.print(n);
        Serial.println(" service(s) found");
        for (int i = 0; i < n; ++i) {
            // Print details for each service found
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(MDNS.hostname(i));
            Serial.print(" (");
            Serial.print(MDNS.IP(i));
            Serial.print(":");
            Serial.print(MDNS.port(i));
            Serial.println(")");
            if (MDNS.hostname(i) == "meteonode") {
                // Stop on first neteonode found.
                meteoNode = MDNS.IP(i);
                meteoNodePort = MDNS.port(i);
                break;
            }
        }
    }

    /* Sync time to local */
    Udp.begin(localPort);
    setSyncProvider(getNtpTime);
    setSyncInterval(3600*36);//Every 36 hours, as adviced by http://www.pool.ntp.org/tos.html

    /**Start the history checking history and history2 files*/
    if (!initializeLogger(fileRead("/history.json"),true)){
        initializeLogger(fileRead("/history2.json"),true);
        lastHistoryBackup = now();
    };


    lastSecond = now()-REFRESH_FREQ; //to force update upon start
}

static uint16_t prev_x = 0xffff, prev_y = 0xffff;

void loop() {
    if (!WiFiReturns()) {
        WiFi.reconnect(); //Connection OFF, try reconnection
    } else {
        //connection ON

        //PRINT NEXT REFRESIH IN..
        if (NEXT_REFRESH_IN != REFRESH_FREQ- (now() - lastSecond)){
            NEXT_REFRESH_IN = REFRESH_FREQ- (now() - lastSecond);
            tft.setTextColor(0X2222,0X0000);
            tft.setTextSize(0.1);
            tft.setCursor(columnSize*5+50, rowSize*4+40);
            tft.print(NEXT_REFRESH_IN);
        }
        /* CONNECTION PART */
        if (now() - lastSecond >= REFRESH_FREQ) {
            //REFRESH_FREQ += REFRESH_FREQ;

            // Get MEteoStation Data
            Serial.print("connecting to ");
            Serial.println(meteoNode.toString());

            HTTPClient http;

            Serial.print("[HTTP] begin...\n");
            http.begin("http://" + meteoNode.toString() + ":" + meteoNodePort + "/getMeteoData?time=now");

            int httpCode = http.GET();

            // timeout handler.
            /*unsigned long timeout = millis();
      while (client.available() == 0) {
      if (millis() - timeout > 10000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
      }*/
            if (httpCode > 0) {
                const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(8) + 130;
                DynamicJsonBuffer jsonBuffer(bufferSize);
                JsonArray& root = jsonBuffer.parseArray(http.getString());
                if (!root.success()) {
                    Serial.println(F("Parsing failed!"));
                    return;
                }
                JsonObject& root_0 = root[0];
                temperaturac = root_0["temperatura"];
                dewPoint = root_0["dew_point"];
                humitat = root_0["humitat"];
                pressio = root_0["pressio"];
                windspeedkph = root_0["wind_speed"];
                windgustdir = root_0["wind_deg"];
                rainin = root_0["rain_1h"];
                dailyrainin = root_0["rain_24h"];

                Serial.print(now());
                Serial.print(root_0.size());
                root_0.prettyPrintTo(Serial);

                // this opens the file "f.txt" to write data
                /*************** PREPEND TO HISTORY ******************/
                WeatherTake currentTake;
                currentTake.name = "take";
                currentTake.pressio = root_0["pressio"];
                currentTake.time = now();
                prependTakeToHistory(currentTake);

                /*************** RETURN TO JSON SERIALIZED TEXT TO SAVE TO FILE ***********************/
                historyToFile();
            }
            http.end();
            Serial.println();
            Serial.println("closing connection");


            //tft.fillScreen(ILI9341_BLACK);
            // Start the rendering:
            int forecast_id;

            if (history[0].pressio>0.0 and history[3].pressio>0.0){
                int Z=calc_zambretti_wind(history[0].pressio,(int)(history[0].pressio-(history[1].pressio+history[2].pressio+history[3].pressio)/3), windgustdir, 10);
                Serial.println(forecast[Z]);
                forecast_id=Z;
            } else {
                //Not enough history to calc zambretti, guess by pressure only
                if (history[0].pressio<1005.00){
                    // 26 pluja
                    Serial.println(forecast[steady_options[26]]);
                    forecast_id=steady_options[26];
                }else if (history[0].pressio>=1005.00  and history[0].pressio <=1015.00){
                    // 8 Cloud
                    Serial.println(forecast[steady_options[8]]);
                    forecast_id=steady_options[8];
                }else if (history[0].pressio>1015.00  and history[0].pressio <1025.00){
                    // 9  Canviable
                    Serial.println(forecast[steady_options[9]]);
                    forecast_id=steady_options[9];
                }else{
                    Serial.println(forecast[steady_options[26]]);
                    forecast_id=steady_options[26];
                }
            }

            tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);

            forecastGraph(tft, 0,0,columnSize*2,rowSize*3, forecast_id);

            windgustdir = windgustdir + 45;
            windDir(tft, 0, rowSize*3, columnSize*2, rowSize*2, windgustdir, windspeedkph);

            tft.setTextColor(ILI9341_ORANGE,ILI9341_BLACK);
            heading(tft, columnSize*2, 0, 3.5, 0, "temperatura", temperaturac);
            tft.setTextColor(ILI9341_BLUE,ILI9341_BLACK);
            heading(tft, columnSize * 4, 0, 3.5, 0, "humitat", humitat);

            tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);
            heading(tft, columnSize*2, rowSize, 2, 5, "pressio", pressio);

            tft.setTextColor(ILI9341_ORANGE,ILI9341_BLACK);
            heading(tft, columnSize *4, rowSize, 2, 5, "punt rosada", dewPoint);

            tft.setTextColor(ILI9341_GREEN,ILI9341_BLACK);
            heading(tft, columnSize*2, rowSize * 2, 2, 0, "vent(km/h)", windspeedkph);
            heading(tft, columnSize * 4, rowSize * 2, 2, 0, "direccio", windgustdir);

            tft.setTextColor(ILI9341_BLUE,ILI9341_BLACK);
            heading(tft, columnSize*2, rowSize * 3, 2, 0, "pluja(1h)", rainin);
            heading(tft, columnSize * 4, rowSize * 3, 2, 0, "pluja(dia)", dailyrainin);

            tft.setTextColor(ILI9341_WHITE,ILI9341_BLACK);

            barGraph(tft, columnSize*2, 190, 180, 50, samples, history);
            tft.setTextColor(0X2222,0x0000);
            tft.setTextSize(0.1);
            tft.setCursor(columnSize*4, rowSize*4);
            tft.print("t:");
            lastSecond = now();

            char dateStr[10];
            sprintf(dateStr, "%02d.%02d.%d", day(lastSecond), month(lastSecond),year(lastSecond));
            tft.print(dateStr);
            tft.print(" ");
            char timeStr[5];
            sprintf(timeStr, "%02d:%02d", hour(lastSecond), minute(lastSecond));
            tft.print(timeStr);
            //delay(2000);

            lastSecond = now();
        }
    }
}

/**
 * @brief getNtpTime
 *  Gets the time from ntpServerName predefined var
 * @return time_t
 *      returns the time.
 */
time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP server's ip address

    while (Udp.parsePacket() > 0) ; // discard any previously received packets
    Serial.println("Transmit NTP Request");
    // get a random server from the pool
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(": ");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500) {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE) {
            Serial.println("Receive NTP Response");
            Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
            unsigned long secsSince1900;
            // convert four bytes starting at location 40 to a long integer
            secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println("No NTP Response :-(");
    return 0; // return 0 if unable to get the time
}

//
/**
 * @brief sendNTPpacket
 *      send an NTP request to the time server at the given address
 * @param address
 *      the IP where to send packet
 */
void sendNTPpacket(IPAddress &address)
{
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    Udp.beginPacket(address, 123); //NTP requests are to port 123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}


/**
 * @brief fileRead
 *      Dumps the contents of a file
 * @param name
 *      the File name
 * @return String
 *      String of the data of SPIFFS
 */
String fileRead(String name) {
    //read file from SPIFFS and store it as a String variable
    String contents;
    File file = SPIFFS.open(name.c_str(), "r");
    if (!file) {
        String errorMessage = "Can't open '" + name + "' !\r\n";
        Serial.println(errorMessage);
        return "FILE ERROR";
    }
    else {

        // this is going to get the number of bytes in the file and give us the value in an integer
        int fileSize = file.size();
        int chunkSize = 1024;
        //This is a character array to store a chunk of the file.
        //We'll store 1024 characters at a time
        char buf[chunkSize];
        int numberOfChunks = (fileSize / chunkSize) + 1;

        int count = 0;
        int remainingChunks = fileSize;
        for (int i = 1; i <= numberOfChunks; i++) {
            if (remainingChunks - chunkSize < 0) {
                chunkSize = remainingChunks;
            }
            file.read((uint8_t *)buf, chunkSize - 1);
            remainingChunks = remainingChunks - chunkSize;
            contents += String(buf);
        }
        file.close();
        return contents;
    }
}

/**
 * @brief fsInit
 *      Initializes the filesystem checking if history is there, if not, creates it
 */
void fsInit(void) {
    // always use this to "mount" the filesystem
    bool result = SPIFFS.begin();
    Serial.println("SPIFFS opened: " + result);

    // this opens the file "f.txt" in read-mode
    f = SPIFFS.open("/history.json", "r");


    while (!f) {
        Serial.println("File doesn't exist yet. Creating it");

        // open the file in write mode
        f = SPIFFS.open("/history.json", "w");
        while (f.available()) {
            //Lets read line by line from the file
            String line = f.readStringUntil('\n');
            Serial.println(line);
        }
        if (!f) {
            Serial.println("file history.json creation failed");
        }
        delay(2000);
    }
    f.close();

    // this opens the secondary file "/history2.json" in read-mode
    fb = SPIFFS.open("/history2.json", "r");

    while (!fb) {
        Serial.println("File doesn't exist yet. Creating it");

        // open the file in write mode
        fb = SPIFFS.open("/history2.json", "w");
        while (fb.available()) {
            //Lets read line by line from the file
            String line = fb.readStringUntil('\n');
            Serial.println(line);
        }
        if (!fb) {
            Serial.println("file history2.json creation failed");
        }
        delay(2000);
    }
    fb.close();
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
