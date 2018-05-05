#ifndef WL_H
#define WL_H
#include <TimeLib.h>
#include <ArduinoJson.h>
#include "weathertake.h"
extern const int older_sample; // The older sample to store in hours
extern const int samples; // Store last 4 samples
extern int updateSamplesFreq; // Update samples every ten takes.
extern time_t timeNow;

extern unsigned long lastHistoryBackup; // To store the last backup execution time
extern int historyBackupFrequency; // how often the backup file must be filled.
extern WeatherTake history[];


bool initializeLogger(String json,bool reset);
void historyToFile();
void addHistoryTakeToJson(JsonArray& root, int i);
void prependTakeToHistory(WeatherTake take);
/*void fillConcurrentSamples(); DEPRECATED*/
bool allocateinTwelvePortion(int, time_t);
bool backupTimeout();
#endif
