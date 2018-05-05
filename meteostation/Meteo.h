/*
 Weather Shield Station
 By: Aleix Quintana Alsius
 Date: May 5th, 2018
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 Much of this is based on Mike Grusin's USB Weather Board code: https://www.sparkfun.com/products/10586 and later Nathan Seidle

 This code reads all the various sensors (wind speed, direction, rain gauge, humidty, pressure, light, batt_lvl)
 and reports it over the serial comm port. This can be easily routed to an datalogger (such as OpenLog) or
 a wireless transmitter (such as Electric Imp).

 Measurements are reported once a second but windspeed and rain gauge are tied to interrupts that are
 calcualted at each report.

 This example code assumes the GPS module is not used.

 */
#ifndef METEO_H
#define ONE_H
#include <Wire.h> //I2C needed for sensors
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Pel sensor de pressio
extern Adafruit_BME280 bme;

//Hardware pin definitions
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
// digital I/O pins
extern const byte WSPEED;
extern const byte RAIN;
extern const byte STAT1;
extern const byte STAT2;

// analog I/O pins
extern const byte REFERENCE_3V3;
extern const byte LIGHT;
extern const byte BATT;
extern const byte WDIR;
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

//Global Variables
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
extern long lastTime; //The millis counter
extern byte seconds; //When it hits 60, increase the current minute
extern byte seconds_2m; //Keeps track of the "wind speed/dir avg" over last 2 minutes array of data
extern byte minutes; //Keeps track of where we are in various arrays of data
extern byte minutes_10m; //Keeps track of where we are in wind gust/dir over last 10 minutes array of data

extern long lastWindCheck;
extern volatile long lastWindIRQ;
extern volatile byte windClicks;

//We need to keep track of the following variables:
//Wind speed/dir each update (no storage)
//Wind gust/dir over the day (no storage)
//Wind speed/dir, avg over 2 minutes (store 1 per second)
//Wind gust/dir over last 10 minutes (store 1 per minute)
//Rain over the past hour (store 1 per minute)
//Total rain over date (store one per day)

extern byte windspdavg[120]; //120 bytes to keep track of 2 minute average

#define WIND_DIR_AVG_SIZE 120
extern int winddiravg[WIND_DIR_AVG_SIZE]; //120 ints to keep track of 2 minute average
extern float windgust_10m[10]; //10 floats to keep track of 10 minute max
extern int windgustdirection_10m[10]; //10 ints to keep track of 10 minute max
extern volatile float rainHour[60]; //60 floating numbers to keep track of 60 minutes of rain

//These are all the weather values that wunderground expects:
extern int winddir; // [0-360 instantaneous wind direction]
extern float windspeedkph; // [kph instantaneous wind speed
extern float windgustkph; // [kph current wind gust, using software specific time period]
extern int windgustdir; // [0-360 using software specific time period]
extern float windspdkph_avg2m; // [mph 2 minute average wind speed mph]
extern int winddir_avg2m; // [0-360 2 minute average wind direction]
extern float windgustkph_10m; // [mph past 10 minutes wind gust mph ] //TODO ->move to km/h
extern int windgustdir_10m; // [0-360 past 10 minutes wind gust direction]

extern float city_meters_above_sea; //TERRASSA + pisos
extern float humitat; // [%]
extern float temperaturaf; // [temperature F]
extern float temperaturac; //  [temperature C]
extern float dew_point;
extern float barometer_raw;
extern float barometer_mB;
extern float pressio;

extern float rainin; // [rain inches over the past hour)] -- the accumulated rainfall in the past 60 min
extern volatile float dailyrainin; // [rain inches so far today in local time]

extern float batt_lvl; //[analog value from 0 to 1023]
extern float light_lvl; //[analog value from 0 to 1023]

// volatiles are subject to modification by IRQs
extern volatile unsigned long raintime, rainlast, raininterval, rain;

void calcWeather();
void rainIRQ();
void wspeedIRQ();
void blinking();

void calcWeather();
float get_light_level();
float get_battery_level();
float get_wind_speed();
float get_wind_speed_ms();
float get_wind_speed_kph();
int get_wind_direction();
float getPressureSeaLevel(float altitude, float temp, float pres);
float getDewPoint(float temp, float hum);
#endif

