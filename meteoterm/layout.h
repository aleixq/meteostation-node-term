#ifndef LAYOUT_H
#define LAYOUT_H

#include "weathertake.h"
#include <Adafruit_ILI9341esp.h>
extern int margin;
extern char* forecast[];

void heading(Adafruit_ILI9341 tft, int x, int y, float scale, int topMargin, char* title, float value);
void barGraph(Adafruit_ILI9341 tft, int x, int y, int w, float h, int smpls, WeatherTake data[]);
void windDir(Adafruit_ILI9341 tft, int x, int y, int w, int h, float dir, float speed);
void testdrawchar(Adafruit_ILI9341 tft);
void forecastGraph(Adafruit_ILI9341 tft, int x, int y, int w, int h, int forecast_id);
#endif // LAYOUT_H
