#include <Arduino.h>
#include "Screen.h"


void delayScroll(int iter){
  for (int i = 0; i < iter ; i++){
    //if ( (i % 4) == 0) { lcd.scrollDisplayLeft(); }
    if ( (i % 20) == 0) { nextScreen(); }
    delay(100);
  }
}

void nextScreen(){
    //Alternating 4 rows if any.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(lcdOut[curScreen]);
    lcd.setCursor(0,1);
    lcd.print(lcdOut[curScreen+1]);
    lcd.setCursor(0, 2);
    lcd.print(lcdOut[curScreen+2]);
    lcd.setCursor(0,3);
    lcd.print(lcdOut[curScreen+3]);
    if (LCD_ROWS_TOPRINT > 4){
      curScreen= curScreen+4;
      if (curScreen == 5){
        curScreen = 0;
      }
    }
}
