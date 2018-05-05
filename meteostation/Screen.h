#ifndef SCREEN_H
#define SCREEN_H
#include <LiquidCrystal_I2C.h>

//Create lcd object with 0x3F direction -> 20 columns x 4 rows
extern LiquidCrystal_I2C lcd;
//LiquidCrystal_I2C lcd(0x27,16,2);

#define LCD_ROWS_TOPRINT 4
extern String lcdOut[LCD_ROWS_TOPRINT];
extern int curScreen;

void delayScroll(int iter);
void nextScreen();

#endif // SCREEN_H
