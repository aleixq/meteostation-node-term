#include "Forecaster.h"
#include <math.h>

/**
 * @brief degToCard
 *      Converts a degree to Cardinal
 * @param deg
 *      The wind direction in degrees.
 * @return
 *      The wind direction in cardinal.
 */
char* degToCard(float deg) {
  while (deg >= 360) {
    deg = deg - 360;
  }
  if (deg > 11.25 && deg < 33.75) {
    return "NNE";
  } else if (deg > 33.75 && deg < 56.25) {
    return "ENE";
  } else if (deg > 56.25 && deg < 78.75) {
    return "E";
  } else if (deg > 78.75 && deg < 101.25) {
    return "ESE";
  } else if (deg > 101.25 && deg < 123.75) {
    return "ESE";
  } else if (deg > 123.75 && deg < 146.25) {
    return "SE";
  } else if (deg > 146.25 && deg < 168.75) {
    return "SSE";
  } else if (deg > 168.75 && deg < 191.25) {
    return "S";
  } else if (deg > 191.25 && deg < 213.75) {
    return "SSW";
  } else if (deg > 213.75 && deg < 236.25) {
    return "SW";
  } else if (deg > 236.25 && deg < 258.75) {
    return "WSW";
  } else if (deg > 258.75 && deg < 281.25) {
    return "W";
  } else if (deg > 281.25 && deg < 303.75) {
    return "WNW";
  } else if (deg > 303.75 && deg < 326.25) {
    return "NW";
  } else if (deg > 326.25 && deg < 348.75) {
    return "NNW";
  } else {
    return "N";
  }
}

/**
 * Calculate zambretti code taking:
 * @param float curr_pressure
 *   The current pressure in hPa
 * @param int trend
 *   The trend to determine falling or rising or steady (usually curr_press - average prev_pressure)
 * @param float wind
 *   The wind angle.
 * @param month
 *   The month.
 *
 * @returns int
 *   code of zambretti to get the value of the array by key.
 */
int calc_zambretti_wind(int curr_pressure, int trend, float wind, int month){
    bool season = (month >= 4 && month <= 9) ; 	// true if 'Summer'
    float low_pressure = 950.00;
    float top_pressure = 1050.00;
    char* z_wind = degToCard(wind);
    //range_correction = (top_pressure-low_pressure/22) HARDCODED to prevent too much computation.
    if (N_HEMISPHERE) {  		// North hemisphere
        // curr_pressure is corrected depending of wind direction: pressure / 100 * (max_presure-min_pressure in the area)
        // Usually max and min are determined as 1050 and min 950 so 100 * 100 = 10000
        if (z_wind == "N") {
            curr_pressure += 6.0 / 10000.0 ;
        } else if (z_wind == "NNE") {
            curr_pressure += 5.0 / 10000.00 ;
        } else if (z_wind == "NE") {
//			curr_pressure += 4.0 ;
            curr_pressure += 5.0 / 10000.00 ;
        } else if (z_wind == "ENE") {
            curr_pressure += 2.0 / 10000.00 ;
        } else if (z_wind == "E") {
            curr_pressure -= 0.5 / 10000.00 ;
        } else if (z_wind == "ESE") {
//			curr_pressure -= 3.0 ;
            curr_pressure -= 2.0 / 10000.00 ;
        } else if (z_wind == "SE") {
            curr_pressure -= 5.0 / 10000.00 ;
        } else if (z_wind == "SSE") {
            curr_pressure -= 8.5 / 10000.00 ;
        } else if (z_wind == "S") {
//			curr_pressure -= 11.0 ;
            curr_pressure -= 12.0/ 10000.00 ;
        } else if (z_wind == "SSW") {
            curr_pressure -= 10.0 / 10000.00 ;  //
        } else if (z_wind == "SW") {
            curr_pressure -= 6.0 / 10000.00 ;
        } else if (z_wind == "WSW") {
            curr_pressure -= 4.5 / 10000.00 ;  //
        } else if (z_wind == "W") {
            curr_pressure -= 3.0 / 10000.00 ;
        } else if (z_wind == "WNW") {
            curr_pressure -= 0.5 / 10000.00 ;
        }else if (z_wind == "NW") {
            curr_pressure += 1.5 / 10000.00 ;
        } else if (z_wind == "NNW") {
            curr_pressure += 3.0 / 10000.00 ;
        }
        if (season) {  	// if Summer
            if (trend == 1) {  	// rising
                curr_pressure += 7.0 / 10000.00;
            } else if (trend == 2) {  //	falling
                curr_pressure -= 7.0 / 10000.00;
            }
        }
    } else {  	// must be South hemisphere
        if (z_wind == "S") {
            curr_pressure += 6.0 / 10000.00 ;
        } else if (z_wind == "SSW") {
            curr_pressure += 5.0 / 10000.00 ;
        } else if (z_wind == "SW") {
//			curr_pressure += 4.0 ;
            curr_pressure += 5.0 / 10000.00 ;
        } else if (z_wind == "WSW") {
            curr_pressure += 2.0 / 10000.00 ;
        } else if (z_wind == "W") {
            curr_pressure -= 0.5 / 10000.00 ;
        } else if (z_wind == "WNW") {
//			curr_pressure -= 3.0 ;
            curr_pressure -= 2.0 / 10000.00 ;
        } else if (z_wind == "NW") {
            curr_pressure -= 5.0 / 10000.00 ;
        } else if (z_wind == "NNW") {
            curr_pressure -= 8.5 / 10000.00 ;
        } else if (z_wind == "N") {
//			curr_pressure -= 11.0 ;
            curr_pressure -= 12.0 / 10000.00 ;
        } else if (z_wind == "NNE") {
            curr_pressure -= 10.0 / 10000.00 ;  //
        } else if (z_wind == "NE") {
            curr_pressure -= 6.0 / 10000.00 ;
        } else if (z_wind == "ENE") {
            curr_pressure -= 4.5 / 10000.00 ;  //
        } else if (z_wind == "E") {
            curr_pressure -= 3.0 / 10000.00 ;
        } else if (z_wind == "ESE") {
            curr_pressure -= 0.5 / 10000.00 ;
        }else if (z_wind == "SE") {
            curr_pressure += 1.5 / 10000.00 ;
        } else if (z_wind == "SSE") {
            curr_pressure += 3.0 / 10000.00 ;
        }
        if (!season) { 	// if Winter
            if (trend == 1) {  // rising
                curr_pressure += 7.0 / 10000.00;
            } else if (trend == 2) {  // falling
                curr_pressure -= 7.0 / 10000.00;
            }
        }
    } 	// END North / South
    //if curr_pressure exceeds 1050 top value, treat as 1049
    //to prevent errors.
    if (curr_pressure == top_pressure){
        curr_pressure = top_pressure-1;
    }
    int option=round((curr_pressure-low_pressure)/4.545); // divided by range_correction
        if (trend >= 3) {
        return rise_options[option];
    }
    else if (trend <= -3){
        return fall_options[option];
    }
    else {
        return steady_options[option];
    }
}
