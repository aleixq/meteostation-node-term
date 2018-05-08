#include <Arduino.h>
#include <FS.h>
#include "WeatherLogger.h"
/**
   * @brief json
   * To test
  const char* json = "[{\"pressio\":1024,\"time\":1524259897},"
                     "{\"pressio\":1024,\"time\":1524259897},"
                     "{\"pressio\":1024,\"time\":1524259897},"
                     "{\"pressio\":1024,\"time\":1524259897}]";
    */

/**
 * @brief initializeLogger
 *      Initialize the history with the data from Flash json or sets it to 0;
 * @param json
 *      The json from which to start the history (if any).
 * @return
 * 		boolean indicating if we have a recovered or false if reseted or initialized a logger
 */
bool initializeLogger(String json,bool reset) {
    // put your setup code here, to run once:
    //Serial.begin(115200);

    /******READ FROM MEMORY THE STORED ARRAY OF HISTORY OR INITIALIZE WITH 0 VALUES ********/
    if (json.length() > 0) {
        const size_t bufferSize = JSON_ARRAY_SIZE(samples) + samples * JSON_OBJECT_SIZE(2) + 130;
        DynamicJsonBuffer jsonBuffer(bufferSize);
        Serial.println(json);
        JsonArray& root = jsonBuffer.parseArray(json);
        if (!root.success()) {
            Serial.println(F("Parsing history from flash failed!"));
            // Set history samples to 0; this will offer the raw barometric history
            for (int i = 0; i < samples; i++) {
                history[i].name = "null";
                history[i].pressio = 0;
                history[i].time = 0;
            }
            return false;
        }else{
            Serial.println(F("Ok! we got history from flash!"));
        }

        // Set history samples to the values from spiffs; this will offer the raw barometric history
        for (int i = 0; i < samples; i++) {
            history[i].name = "take";
            history[i].pressio = root[i]["pressio"];
            history[i].time = root[i]["time"];
            return true;
        }
    }
    if (reset){
        // Set history samples to 0; this will offer the raw barometric history
        for (int i = 0; i < samples; i++) {
            history[i].name = "null";
            history[i].pressio = 0;
            history[i].time = 0;
        }
        return false;
    }
}

/**
 * @brief loopDemo
 *      To test the behavior of logger in a separated sketch
 */
void loopDemo() {
    // put your main code here, to run repeatedly:
    //timeNow += millis() / 1000;


    /******ADD NEW SENSOR AND PREPEND! TO HISTORY                **********************/
    const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(8) + 130;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonArray& root = jsonBuffer.parseArray("[{ \"dew_point\": 10.0, \"temperatura\" : 11.0, \"pressio\" : 1024.0, \"humitat\" : 40.0, \"wind_speed\" : 12.0, \"wind_deg\" : 120, \"rain_1h\" : 13.0, \"rain_24h\" : 14.0}]");

    if (!root.success()) {
        Serial.println(F("Parsing failed!"));
        return;
    }
    JsonObject& root_0 = root[0];
    // add SNTP (or RTC, or millis)

    /*************** PREPEND TO HISTORY ******************/
    WeatherTake currentTake;
    currentTake.name = "take";
    currentTake.pressio = root_0["pressio"];
    currentTake.time = timeNow;
    prependTakeToHistory(currentTake);

    /*************** RETURN TO JSON SERIALIZED TEXT TO SAVE TO FILE ***********************/
    historyToFile();

    //delay(updateSamplesFreq * 1000);
}

/**
 * @brief historyToFile
 *      Exports the history to the flash file in esp8622
 */
void historyToFile() {
    File f;
    const size_t bufferSize_h = JSON_ARRAY_SIZE(samples) + samples * JSON_OBJECT_SIZE(2);
    DynamicJsonBuffer jsonBuffer_h(bufferSize_h);

    JsonArray& root_h = jsonBuffer_h.createArray();
    for (int i = 0; i < samples; i++) {
        addHistoryTakeToJson(root_h, i);
    }
    Serial.println("Printing the history by now:");
    root_h.prettyPrintTo(Serial);

    // this opens the file "f.txt" to write data
    f = SPIFFS.open("/history.json", "w");
    root_h.prettyPrintTo(f);
    f.println();
    f.close();
    if (backupTimeout()){
        Serial.println("Backing up the history");
        File fb;
        // this opens the file "f.txt" to write data
        fb = SPIFFS.open("/history2.json", "w");
        root_h.prettyPrintTo(f);
        fb.println();
        fb.close();
        lastHistoryBackup = now();
    }
}

/**
 * @brief addHistoryTakeToJson
 *      Add a single weather take to JSon used to store the history in a file.
 * @param root
 *      The target json array to add a take.
 * @param i
 *      The index of the history to add.
 */
void addHistoryTakeToJson(JsonArray& root, int i) {
    JsonObject& root_x = root.createNestedObject();
    root_x["pressio"] = history[i].pressio;
    root_x["time"] = history[i].time;
}

/**
 * @brief prependTakeToHistory
 *    Prepends a weather take to the history array at position 0.
 *    Move each item key one place to make allocate space
 *    for new weather take.
 * @param take
 *    The pressure takes.
 */
void prependTakeToHistory(WeatherTake take) {
    /*Serial.println("===>Trying to allocate this take:");
    Serial.print("time: ");Serial.println(take.time);
    Serial.print("pressio: ");Serial.println(take.pressio);
    Serial.print("Now time is: ");Serial.println(now());
    Serial.println("Now it will check every history values starting from key 3 and descending to check if they can be allocated in any period range and so on be shifted: ");
    */
    for (int i = samples - 1; i > 0; i--){
        /**
        Same as:
          history[3] = history[2];
          history[2] = history[1];
          history[1] = history[0];
    **/
        //Serial.print("================== > Checking for a candidate for position ");Serial.println(i);
        //Serial.print("It must be between: ");Serial.print((older_sample/samples)*i*10); Serial.print(" and: ");Serial.println(((older_sample/samples)*i)*10+(older_sample/samples)*10);
        bool overriden = false;
        for (int ri = i; ri > -1; ri--){//recursive indent to find range candidate(starting at i-1)
            //Serial.print("Below candidate time from ri: ");Serial.print(ri);Serial.print(" is ");Serial.println(now()-history[ri].time);
            //Check all values in inferior array keys if they can be in current range.
            time_t timeDiff = now() - history[ri].time ; // set timediff to now - the value that will override current(next candidate starting at i-1 key in array)
            if (allocateinTwelvePortion(i, timeDiff)){
                history[i] = history[ri];
                //Serial.print("Shifting ri pos "); Serial.print(ri);Serial.print("  "); Serial.print(" to position ");Serial.println(i);
                overriden = true;
                break;
            }
        }
        //in case of not overriden set values of period to 0
        if (!overriden){
            //Serial.print("Cannot found a valid take from any saved takes to occupy this position, set to zero i :");Serial.println(i);
            history[i].time = 0;
            history[i].pressio = 0.0;
        }
    }
    // ONLY RESET VALUE if IS GREATER THAN A SAMPLE PERIOD
    if (take.time-history[0].time >= ((older_sample/samples)*3600)) { // to test set 30 instead of 3600
        //Serial.println("===========Current Take allocated at 0=============");
        history[0] = take;
    } else{
        //Serial.print("Next history keys moveup will occur when(ms): ");Serial.println(take.time-history[0].time);
        //Serial.print("is greater than "); Serial.println((older_sample/samples)*10);
    }
    /*DEBUG*/
/*for (int i = 0; i <= 3; i++){
    Serial.print("history [");Serial.print(i);Serial.print("] rang >= ");Serial.print((older_sample/samples)*i*10); Serial.print(" i <");Serial.println(((older_sample/samples)*i)*10+(older_sample/samples)*10);
    Serial.print("	Temps: ");Serial.println(now()-history[i].time);
    Serial.print("	pressio: ");Serial.println(history[i].pressio);
}*/
}

/**
 * @brief allocateinTwelvePortion
 *      Calculates the range of the position inside older_sample hours, and then checks if needle time is inside this range.
 * @param portion_position
 *      portion position inside older_sample hours.
 * @param needle
 *      C time to check
 * @return bool
 *      If the needle is in the portion interval specified
 */
bool allocateinTwelvePortion(int portion_position, time_t needle){
    // We need to be permisive with last take as sometimes the take exceeds the range depending on var REFRESH_FREQ,
    // if REFRESH_FREQ + Computing time is not multiple of last history take top_time the last take will be discarded:
    // Imagine a REFRESH_FREQ of 7 seconds (+1 second of connecting/computing). The shifting operation will be checked
    // each 8 seconds, so a take of 120 seconds ago will not be in the top_time of 120 and so on will be discarded
    // 0-29	30-59	60-89	90-119   -> History ranges in seconds
    // ---------------------------
    // 0	1		2		3
    // 0	32		64		96
    // 8	40		72		104
    // 16	48		80		112
    // 24	56		88
    // 0	32		64		96
    // 8	40		72		104
    // 16	48		80		112
    // 24	56		88
    // The solution to this is to double the top_time in the last range (anyway more old data is better to compute zambretti and pressure evolution).

    int low_time =(older_sample/samples)*portion_position*3600; // to test set 10 instead of 3600
    int top_time = ((older_sample/samples)*portion_position)*3600+(older_sample/samples)*3600; // to test set 10 instead of 3600
    // be permissive as explained above.
    if (portion_position==samples-1)
        top_time += (older_sample/samples)*3600;// to test set 10 instead of 3600
    Serial.print("The range in the checked portion ");Serial.print(portion_position);Serial.print(" is: ");Serial.print(low_time);Serial.print("  ~  ");Serial.println(top_time);
    return needle >= low_time && needle < top_time;
}

/**
 * @brief backupTimeout
 * 		Checks if now it is out of time to proceed with a backup
 * @return
 * 		True if it is timeout
 */
bool backupTimeout(){
    return now()-lastHistoryBackup > historyBackupFrequency;
}
