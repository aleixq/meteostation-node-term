#include "layout.h"
#include <Adafruit_ILI9341esp.h>
#include "Forecaster.h"
#include "wu_icons.h"

/**
 * @brief heading
 *      Creates a Key -> value rendering box.
 * @param tft
 *      A ILI9341 display object where to render Graphic.
 * @param x
 *      X position.
 * @param y
 *      Y position.
 * @param scale
 *      The scaled factor size of the value.
 * @param topMargin
 *      The top margin to apply to the whole box
 * @param title
 *      The title of the value.
 * @param value
 *      The value of the title.
 */
void heading(Adafruit_ILI9341 tft, int x, int y, float scale, int topMargin, char* title, float value) {
  tft.setCursor(margin + x, topMargin + y);
  if (scale - 2 < 1) {
    tft.setTextSize(1); // prevent 0 size
  } else {
    tft.setTextSize(scale - 2);
  }
  tft.println(title);
  tft.println("");
  tft.setTextSize(scale);
  tft.setCursor(margin + x, tft.getCursorY());
  tft.println(value);
}

/**
 * @brief barGraph
 *      Renders a pressure history in form of bar graphs.
 * @param tft
 *      A ILI9341 display object where to render Graphic.
 * @param x
 *      X position.
 * @param y
 *      Y position.
 * @param w
 *      Width.
 * @param h
 *      height.
 * @param smpls
 *      Number of samples to display.
 * @param data
 *      Array of pressure values.
 */
void barGraph(Adafruit_ILI9341 tft, int x, int y, int w, float h, int smpls, WeatherTake data[]) {
  int bar_width = w / smpls;
  int relative_y = y + h;
  //Print concurrent baro to tft
  char buffer[6] = " ";
  tft.fillRect(x, y, w, h, ILI9341_BLACK);

  for (int i = 0; i < smpls; i++) {
    //Serial.println(history[i].pressio);
    //tft.print(dtostrf(history[i].pressio, 4, 2, buffer));
    //tft.print(",");

    // comptem que la finestra de mBars va de 950 a 1050 (100) corregim la unitat d'alcada.
    float ranged_hunit = h / 100;
    int crop_pressio = data[i].pressio - 950;
    float scaled_pressio = crop_pressio * ranged_hunit;
    if (crop_pressio > 0 ) {
      tft.drawFastHLine(x+w - bar_width - (i * bar_width), relative_y - scaled_pressio, bar_width, 0xFFFF );
      //set the graph markers, (x[i*width+margin], y[ (height)pressure+offset + margin]
      tft.setCursor(x+w - bar_width - (i * bar_width) + 2, relative_y - ranged_hunit * 50 );
      tft.setTextSize(0.1);
      tft.print(data[i].pressio);
      tft.println("");
      tft.setCursor(x+w - bar_width - (i * bar_width) + 2, relative_y - 10);
      //print the range
      int top_time = (12 / smpls) * i + 1;
      tft.print(top_time); tft.print("h.");

    }
  }
}

/**
 * @brief windDir
 *      Renders a polar graph of the wind data (speed and direction).
 * @param tft
 *      A ILI9341 display object where to render Graphic.
 * @param x
 *      X position.
 * @param y
 *      Y position.
 * @param w
 *      Width.
 * @param h
 *      height.
 * @param dir
 *      The direction of wind in degrees.
 * @param speed
 *      The wind speed in m/s.
 */
void windDir(Adafruit_ILI9341 tft, int x, int y, int w, int h, float dir, float speed) {
  //tft.drawRect(x, y, w, h, 0xFFFF);
  int radius = ((x>y)?w / 2:h/2)-2; // -2 is margin
  ////////////////PARTS OF THE ANALOG CLOCK THAT WILL NOT BE MOVING////////////////

  float x1, y1, x2, y2, b;
  const float pi = 3.14;
  b = ((dir - 90) * pi) / 180;

  // tick radius length:
  // origin + and length over a radius of a circle
  float tick_percent_start = w / 2 * 0.7;
  float tick_percent_length = w / 2 * 0.3;
  x1 = x + radius + (tick_percent_start * cos(b));
  y1 = y + radius + (tick_percent_start * sin(b));
  x2 = x1 + (tick_percent_length * cos(b));
  y2 = y1 + (tick_percent_length * sin(b));

  Adafruit_GFX_Button currentDir;
  //initButton(Adafruit_GFX *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, int16_t textcolor, char *label, uint8_t textsize);
  currentDir.initButton(&tft, x + radius, y + radius , w, h, ILI9341_BLACK, ILI9341_BLACK, ILI9341_GREEN, degToCard(dir) , 3);
  currentDir.drawButton();

  Adafruit_GFX_Button currentSpeed;
  char speed_str[6];
  //initButton(Adafruit_GFX *gfx, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t outline, uint16_t fill, int16_t textcolor, char *label, uint8_t textsize);
  currentSpeed.initButton(&tft, x + radius, y + radius + (radius / 2) , w, 20, ILI9341_BLACK, ILI9341_BLACK, ILI9341_GREEN, dtostrf(speed, 3, 2, speed_str), 1);
  currentSpeed.drawButton();

  tft.setTextColor(ILI9341_GREEN);
  tft.setCursor(radius - 5, y);
  tft.setTextSize(2);
  tft.println("N");

  tft.drawLine(x1, y1, x2, y2, 0xF000);
  tft.drawCircle(x + radius, y + (h / 2), radius, ILI9341_GREEN);

}


/**
 * @brief testdrawchar
 *      Tests some renders in display.
 * @param tft
 *      A ILI9341 display object where to render Graphic.
 *
 */
void testdrawchar(Adafruit_ILI9341 tft) {
  // Per importar un bitmap agafem una imatge que estgui en b/w i la passem a mapa de bits de X
  // amb el gimp.
  // Workflow,1- obre imatge, 2- escala acom vulguis, 3-aplana-la per evitar alpha i capes, 4-converteix a blanc i negre (opcional)
  // 5- exporta a xbm i 6- enganxa l'array a la capcalera de cpp que toqui.
  // substituint:
  //    - %s/static/const/g
  //    - %s/_bits//g
  //    - %s/\[\]/[] PROGMEM/g
  // 7 crida aqui (amb els mateixos pixels amb drawXBitmap.

  tft.drawXBitmap(0, 0, sunny, 92, 92,  ILI9341_YELLOW);

  //RAIN
  tft.drawXBitmap(90, 0, rain_cloud, 92, 92,  ILI9341_WHITE);
  tft.drawXBitmap(90, 0, rain_rain, 92, 92,  ILI9341_BLUE);

  //WORSENING
  tft.drawXBitmap(0, 90, worsening_cloud, 92, 92,  ILI9341_WHITE);
  tft.drawXBitmap(0, 90, worsening_sun, 92, 92,  ILI9341_YELLOW);

  //MOSTLY SUN
  tft.drawXBitmap(90, 90, mostlysunny_cloud, 92, 92,  ILI9341_WHITE);
  tft.drawXBitmap(90, 90, mostlysunny_sun, 92, 92,  ILI9341_YELLOW);

  //CLOUD
  tft.drawXBitmap(90, 170, cloudy, 92, 92,  ILI9341_WHITE);

  //STORMY
  tft.drawXBitmap(90, 0, rain_cloud, 92, 92,  ILI9341_WHITE);
  tft.drawXBitmap(90, 0, tstorms_rain, 92, 92,  ILI9341_BLUE);
  tft.drawXBitmap(90, 90, tstorms_lightning, 92, 92,  ILI9341_YELLOW);

  //SHOWER
  tft.drawXBitmap(90, 0, rain_cloud, 92, 92,  ILI9341_WHITE);
  tft.drawXBitmap(90, 0, shower_rain, 92, 92,  ILI9341_BLUE);



  //tft.setTextSize(1);
  //tft.setTextColor(ILI9341_BLACK);
  //tft.setCursor(12, 24);
  //tft.print(String(frame.rain));
  return;

}


/**
 * @brief forecastGraph
 *      Creates a Graphic of the forecast following Zambretti Algorithm
 * @param tft
 *      A ILI9341 display object where to render Graphic
 * @param x
 *      X position
 * @param y
 *      Y position
 * @param w
 *      Width
 * @param h
 *      height
 * @param forecast_id
 *      the Z code of zambretti forecast.
 */
void forecastGraph(Adafruit_ILI9341 tft, int x, int y, int w, int h, int forecast_id) {
    // Due to hardcoded size of icons width and height both w and h needs to be more than 92
    //tft.drawRect(x, y, w, h, 0xFFFF);
    // images are hardcoded at 92...
    int center_pos_x = x + ((w - 92) / 2);
    int center_pos_y = y + ((h - 92) / 2);

    tft.fillRect(center_pos_x, center_pos_y, 92, 92,  ILI9341_BLACK);

    if (forecast_id == 0 or forecast_id == 1) {
        //SUN
        tft.drawXBitmap(center_pos_x, center_pos_y, sunny, 92, 92,  ILI9341_YELLOW);
    }
    if (forecast_id == 2 or forecast_id == 3 or forecast_id == 5 or forecast_id == 6 or forecast_id == 10 or forecast_id == 12 or forecast_id == 13 or forecast_id == 17) {
        //MOSTLY SUN
        tft.drawXBitmap(center_pos_x, center_pos_y, mostlysunny_cloud, 92, 92,  ILI9341_WHITE);
        tft.drawXBitmap(center_pos_x, center_pos_y, mostlysunny_sun, 92, 92,  ILI9341_YELLOW);
    }
    if (forecast_id == 9 or forecast_id == 21) {
        //WORSENING
        tft.drawXBitmap(center_pos_x, center_pos_y, worsening_cloud, 92, 92,  ILI9341_WHITE);
        tft.drawXBitmap(center_pos_x, center_pos_y, worsening_sun, 92, 92,  ILI9341_YELLOW);
    }
    if (forecast_id == 20) {
        //CLOUD
        tft.drawXBitmap(center_pos_x, center_pos_y, cloudy, 92, 92,  ILI9341_WHITE);
    }
    if (forecast_id == 4 or forecast_id == 7 or forecast_id == 8 or forecast_id == 11 or forecast_id == 14 or forecast_id == 15 or forecast_id == 16 or forecast_id == 18 or forecast_id == 19 or forecast_id == 21 or forecast_id == 22 or forecast_id == 23 or forecast_id == 24) {
        //SHOWER
        tft.drawXBitmap(center_pos_x, center_pos_y, rain_cloud, 92, 92,  ILI9341_WHITE);
        tft.drawXBitmap(center_pos_x, center_pos_y, shower_rain, 92, 92,  ILI9341_BLUE);
    }
    if (forecast_id == 25 or forecast_id == 26 ) {
        //STORMY
        tft.drawXBitmap(center_pos_x, center_pos_y, rain_cloud, 92, 92,  ILI9341_WHITE);
        tft.drawXBitmap(center_pos_x, center_pos_y, tstorms_rain, 92, 92,  ILI9341_BLUE);
        tft.drawXBitmap(center_pos_x, center_pos_y, tstorms_lightning, 92, 92,  ILI9341_YELLOW);
    }
    tft.setCursor( x+5, y);
    tft.setTextSize(0.2);
    tft.println(forecast[forecast_id]);
}
