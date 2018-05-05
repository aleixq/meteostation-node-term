/*
 Weather Shield Station
 By: Aleix Quintana Alsius
 Date: May 5th, 2018
 Weather shield to serial to comunicate with esp8266 to propagate data
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Much of this is based on Mike Grusin's USB Weather Board code: https://www.sparkfun.com/products/10586, and later Nathan Seidle

 This code reads all the various sensors (wind speed, direction, rain gauge, humidty, pressure, light, batt_lvl)
 and reports it over the serial comm port. This can be easily routed to an datalogger (such as OpenLog) or
 a wireless transmitter (such as Electric Imp).

 Measurements are reported once a second but windspeed and rain gauge are tied to interrupts that are
 calcualted at each report.

 This example code assumes the GPS module is not used.

*/
#include <SoftwareSerial.h>
#include <Arduino.h>
#include <Wire.h>
#include "Screen.h"
#include "Meteo.h"

const byte rxPin = 15; // Wire this to Tx Pin of ESP8266
const byte txPin = 14; // Wire this to Rx Pin of ESP8266

// We'll use a software serial interface to connect to ESP8266
SoftwareSerial ESP8266 (rxPin, txPin);




#define FAKE_WINDIRQ  //TODO , IRQ REMAP  because we use CC3000, look for "TODO" to watch how.
int DEBUG = 1;      // DEBUG counter; if set to 1, will write values back via serial


#include <avr/wdt.h> //We need watch dog for this program


#define IDLE_TIMEOUT_MS  3000 // Amount of time to wait (in milliseconds) with no data
// received before closing the connection.  If you know the server
// you're accessing is quick to respond, you can reduce this value.

#define REFRESH_DATA 1000
#define REFRESH_LCD 5000

/** SCREEN VARS **/
long lastTimeLcd; //The millis lcd counter
//Create lcd object with 0x3F direction -> 20 columns x 4 rows
LiquidCrystal_I2C lcd(0x3F, 20, 4);
//LiquidCrystal_I2C lcd(0x27,16,2);

#define LCD_ROWS_TOPRINT 4
String lcdOut[LCD_ROWS_TOPRINT] = { "T-Pr-W", "VENT", "H-PL", "conMSG" };
int curScreen = 0;
/** END OF SCREEN VARS **/


/* WEATHER VARS */
// Pressure sensor
Adafruit_BME280 bme;
//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// digital I/O pins
const byte WSPEED = 3;
//const byte WSPEED = 19; //if we need any sensor using hardware IRQ(as wifi cc3000) we can change the PIN here.
//const byte WSPEED = 4; // if we want to simulate the IRQ define a fake irq.
const byte RAIN = 2;
const byte STAT1 = 7;
const byte STAT2 = 8;

// analog I/O pins
const byte REFERENCE_3V3 = A3;
const byte LIGHT = A1;
const byte BATT = A2;
const byte WDIR = A0;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Global Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
long lastTime; //The millis counter
byte seconds; //When it hits 60, increase the current minute
byte seconds_2m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
byte minutes; //Keeps track of where we are in various arrays of data
byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data

long lastWindCheck = 0;
volatile long lastWindIRQ = 0;
volatile byte windClicks = 0;

//We need to keep track of the following variables:
//Wind speed/dir each update (no storage)
//Wind gust/dir over the day (no storage)
//Wind speed/dir, avg over 2 minutes (store 1 per second)
//Wind gust/dir over last 10 minutes (store 1 per minute)
//Rain over the past hour (store 1 per minute)
//Total rain over date (store one per day)

byte windspdavg[120]; //120 bytes to keep track of 2 minute average

#define WIND_DIR_AVG_SIZE 120
int winddiravg[WIND_DIR_AVG_SIZE]; //120 ints to keep track of 2 minute average
float windgust_10m[10]; //10 floats to keep track of 10 minute max
int windgustdirection_10m[10]; //10 ints to keep track of 10 minute max
volatile float rainHour[60]; //60 floating numbers to keep track of 60 minutes of rain

//These are all the weather values that wunderground expects:
int winddir = 0; // [0-360 instantaneous wind direction]
float windspeedkph = 0; // [kph instantaneous wind speed
float windgustkph = 0; // [kph current wind gust, using software specific time period]
int windgustdir = 0; // [0-360 using software specific time period]
float windspdkph_avg2m = 0; // [mph 2 minute average wind speed mph]
int winddir_avg2m = 0; // [0-360 2 minute average wind direction]
float windgustkph_10m = 0; // [mph past 10 minutes wind gust mph ] //TODO ->move to km/h
int windgustdir_10m = 0; // [0-360 past 10 minutes wind gust direction]

float city_meters_above_sea = 280.0+10.0; //TERRASSA + pisos
float humitat = 0; // [%]
float temperaturaf = 0; // [temperature F]
float temperaturac = 0; //  [temperature C]
float dew_point;
float barometer_raw = 0.0;
float barometer_mB = 1020.0;
float pressio = 0;

float rainin = 0; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
volatile float dailyrainin = 0; // [rain inches so far today in local time]

float batt_lvl = 11.8; //[analog value from 0 to 1023]
float light_lvl = 455; //[analog value from 0 to 1023]

// volatiles are subject to modification by IRQs
volatile unsigned long raintime, rainlast, raininterval, rain;

/* END OF WEATHER VARS */


namespace
{
  template<typename Any>
  /**
   Look if DEBUG is on and output to std out.
  */
  void debugit(Any value) {
    if (DEBUG) {
      Serial.println(value);
    }
  }
}

void setup(void)
{
  wdt_reset(); //Pet the dog
  wdt_disable(); //We don't want the watchdog during init

  // Init LCD
  lcd.init();
  lcd.init();
  lcd.backlight();
  // Write to LCD.
  lcd.setCursor(0, 0);
  lcd.print("Starting");
  lcd.setCursor(0, 1);
  lcd.print("MeteoStation...");
  /**
     BLINK TO SHOW IT'S WORKING
  */
  pinMode(STAT1, OUTPUT); //Status LED Blue
  blinking();

  Serial.begin(115200);
  ESP8266.begin(9600); // Change this to the baudrate used by ESP8266
  delay(1000); // Let the module self-initialize

  /*
     2- WEATHER SETUP!
  */

  pinMode(STAT1, OUTPUT); //Status LED Blue
  pinMode(STAT2, OUTPUT); //Status LED Green

  pinMode(WSPEED, INPUT_PULLUP); // input from wind meters windspeed sensor
  pinMode(RAIN, INPUT_PULLUP); // input from wind meters rain gauge sensor

  pinMode(REFERENCE_3V3, INPUT);
  pinMode(LIGHT, INPUT);

  //Configure the pressure sensor
  // Barometer bme280 fent servir adafruit llavors hem de canviar l'adreca i2c de 0x77 a 0x76.
  // If using Sparkfun then we must edit the library SparkFun_BME280_Arduino_Library-2.0.0/src/SparkFunBME280.cpp and define settings.I2CAddress = 0x76;
  if (!bme.begin(0x76))
  {
    Serial.println("Error! No BMP Sensor Detected!!!");
    while (1);
  }

  seconds = 0;
  lastTime = millis()-REFRESH_DATA; //Force get sensors upon Start
  lastTimeLcd = millis()-REFRESH_LCD; //Force send data upon Start

  // attach external interrupt pins to IRQ functions
  attachInterrupt(digitalPinToInterrupt(RAIN), rainIRQ, FALLING);
  attachInterrupt(digitalPinToInterrupt(WSPEED), wspeedIRQ, FALLING);

  // turn on interrupts
  interrupts();
}


void loop()
{
  wdt_reset(); //Pet the dog
  /* SENSORS */
  //Keep track of which minute it is
  if (millis() - lastTime >= REFRESH_DATA)
  {
    digitalWrite(STAT1, HIGH); //Blink stat LED

    debugit("Sensors fire!!");
    lastTime = millis();

    //Take a speed and direction reading every second for 2 minute average
    if (++seconds_2m > 119) seconds_2m = 0;

    //Calc the wind speed and direction every second for 120 second to get 2 minute average
    float currentSpeed = windspeedkph;
    int currentDirection = get_wind_direction();
    windspdavg[seconds_2m] = (int)currentSpeed;
    winddiravg[seconds_2m] = currentDirection;

    //Check to see if this is a gust for the minute
    if (currentSpeed > windgust_10m[minutes_10m])
    {
      windgust_10m[minutes_10m] = currentSpeed;
      windgustdirection_10m[minutes_10m] = currentDirection;
    }

    //Check to see if this is a gust for the day
    if (currentSpeed > windgustkph)
    {
      windgustkph = currentSpeed;
      windgustdir = currentDirection;
    }

    if (++seconds > 59)
    {
      seconds = 0;

      if (++minutes > 59) minutes = 0;
      if (++minutes_10m > 9) minutes_10m = 0;

      rainHour[minutes] = 0; //Zero out this minute's rainfall amount
      windgust_10m[minutes_10m] = 0; //Zero out this minute's gust
    }

    //Check to see if there's been lighting
    #ifdef FAKE_WINDIRQ
    if (digitalRead(WSPEED) == HIGH)
    {
      //We've got something!
      wspeedIRQ();
    }
    #endif

    //Report all readings every second
    calcWeather();
    digitalWrite(STAT1, LOW); //Turn off stat LED
  }
  if (millis()- lastTimeLcd >= REFRESH_LCD){
    digitalWrite(STAT2, HIGH); //Start sending Data via serial GREEN LED ON
    //Send data to Serial
    debugit("We got data, start connecting");
    lastTimeLcd = millis();

    // Ship it!
    // To Standard out
    String strOut = String(
                      "[{ \"dew_point\": "  + String(dew_point) +
                      ", \"temperatura\" : " + String(temperaturac) +
                      ", \"pressio\" : " + String(pressio) +
                      ", \"humitat\" : " + String(humitat) +
                      ", \"wind_speed\" : " + String(windspeedkph) +
                      ", \"wind_deg\" : " + String(winddir) +
                      ", \"rain_1h\" : " + String(rainin) +
                      ", \"rain_24h\" : " + String(dailyrainin) +
                      "}]");
    ESP8266.println(strOut);

    debugit(strOut);

    lcdOut[0] = String(
                  "DP"  + String(dew_point) +
                  " " + String(temperaturac) + (char)223);
    lcdOut[1] = String(
                  String(pressio) + "h " +
                  String(humitat) + "%" );
    lcdOut[2] = String(
                  "VENT: " + String(windspeedkph) +
                  " - " + String(winddir) + (char)223);
    lcdOut[3] = String(
                  "P 1h:" + String(rainin) +
                  " 24h:" + String(dailyrainin) +
                  "");

    debugit("--TODO CHECK get_wind_speed and get_windsepeed_v2 (pass to ms) --");
    debugit("Upload complete");

    /* Read data until either the connection is closed, or the idle timeout is reached.
    */
    Serial.flush();


    debugit("SUCCESS! ");
    digitalWrite(STAT2, LOW); //Success!! Turn off GREEN LED
    nextScreen();
  }
  //delayScroll(74); //(5+2.4 seconds)
  //lpDelay(1200); // Low Power Delay. Value of 4=1sec, 40=10sec, 1200=5min --If battery, send every 5 min.
}
