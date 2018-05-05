#ifndef FORECASTER_H
#define FORECASTER_H

#include "weathertake.h"
extern int rise_options[];
extern int fall_options[];
extern int steady_options[];
extern const bool N_HEMISPHERE;
char* degToCard(float deg);
int calc_zambretti_wind(int curr_pressure, int trend, float wind, int month);
int calc_zambretti(int curr_pressure, int prev_pressure, int mon);
#endif // FORECASTER_H
