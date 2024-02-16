#ifndef LOG_MOD_H
#define LOG_MOD_H
#include "st.h"
// includes
#include "storage_mod.h"
#include "clock_mod.h"
#include "config_mod.h"
// log filename format gmt year month day
#define LOG_FORMAT_ "/logs/log_gmt%d_%d_%d_%d.log"
#define LOGS_DIR "/logs"
#define LOG_DELAY 60000
// interface
// variables
static modState LOG_state; // module state
static char logLine[1024]; // line to log
static char logName[64]; // line to log
static int log_delay = LOG_DELAY; // line to log
extern DynamicJsonDocument json_conf;
                           
// functuions
/**
 * @brief Write int value in the current instance of the log string 
 *
 * @param id identifier for the log string
 * @param value int
 */
void LOG_write_int(const char* id, int value){
    sprintf(logLine, "%s%s:%d ", logLine,id,value);
}
/**
 * @brief Write float value in the current instance of the log string 
 *
 * @param id identifier for the log string
 * @param value float
 */
void LOG_write_float(const char* id, float value){
    sprintf(logLine, "%s%s:%f ", logLine,id,value);
}
/**
 * @brief Write string  value in the current instance of the log string 
 *
 * @param id identifier for the log string
 * @param value float
 */
void LOG_write_string(const char* id, String value){
    sprintf(logLine, "%s%s:%s ", logLine,id,value.c_str());
}
// ------------------------
// base interface 
// ------------------------

/**
 * @brief Intializing the log module, make sure sd card module is.
 *
 * @return Module state
 */
modState LOG_init() { 
#ifdef SD_STORAGE_MOD_H
    if (SD_STORAGE_state) {
        SD_STORAGE_init();
        if (SD_STORAGE_state)
            goto failed;
    }
    createDir(SD,LOGS_DIR);
    log_delay = json_conf["log_delay"];
    if (log_delay == 0){
        log_delay = LOG_DELAY;
    }
failed:
    LOG_state = SD_STORAGE_state;
#else
    LOG_state = ModOK;
#endif
    return LOG_state;
}

/**
 * @brief Write the current log line to log file
 *
 * @return Module state
 */
modState LOG_log(){
    int gmt,day,month,year;
    gmt = get_gmt();
    day = get_day();
    month = get_month();
    year = get_year();
    memset(logName, 0, 64);
    sprintf(logName, LOG_FORMAT_, gmt,day,month,year);
    sprintf(logLine, "%s\n",logLine);
    /* Serial.printf("logging in lognme: %s -> %s\n",logName,logLine); */
#ifdef SD_STORAGE_MOD_H
    if (!SD_STORAGE_state)
        appendFile(SD, logName,logLine);
#endif
    memset(logLine, 0, 1024);
    return LOG_state;
}

#endif // MD_MOD_H
