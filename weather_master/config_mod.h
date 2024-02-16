// this "class" would jsut handle writing and reading of configuration
// we dont check the keys and provide a default implementation
// default implementation is best done in place if key is missing
// just so modules are decoupled
#ifndef CONFIG_MOD_H
#define CONFIG_MOD_H
#include "st.h"
#include <ArduinoJson.h>
// includes
#include "storage_mod.h"
#include "clock_mod.h"
#define CONFIG_NAME "/conf.json"
// this is overkill for size of buffer
#define JSON_BUFSIZE 2048 
static char config_buf[JSON_BUFSIZE];
static char config_buf_write[JSON_BUFSIZE];
DynamicJsonDocument json_conf(JSON_BUFSIZE);
// CONFIG filename format gmt year month day
// interface
// variables
static modState CONFIG_state = ModUninitialized; // module state
// functuions
// ------------------------
// base interface 
// ------------------------

/**
 * @brief Read config stored in json file on the sd card
 *
 * @return module state
 */
modState CONFIG_init() {
    if (CONFIG_state) {
#ifdef SD_STORAGE_MOD_H
        if (SD_STORAGE_state) {
            SD_STORAGE_init();
            if (SD_STORAGE_state)
                CONFIG_state = ModError;
        }
        CONFIG_state = ModOK;
        int ret = readFileToBuf(SD,CONFIG_NAME, config_buf, JSON_BUFSIZE);
        if(ret) {
            Serial.printf("Error reading file %s with %d",CONFIG_NAME,ret);
            CONFIG_state = ModError;
        }
        DeserializationError err = deserializeJson(json_conf, config_buf);
        if (err) {
            CONFIG_state = ModError;
        }
#else
        CONFIG_state = ModOK;
#endif // SD_STORAGE_MOD_H
    }
    return CONFIG_state;
}

/**
 * @brief Flush config to sd card
 *
 * @return [TODO:return]
 */
modState CONFIG_update() {
#ifdef SD_STORAGE_MOD_H
    memset(config_buf_write,0,2048);
    serializeJson(json_conf,config_buf_write,2048);
    writeFile(SD,CONFIG_NAME,config_buf_write);
#endif // SD_STORAGE_MOD_H
    return CONFIG_state;
}

#endif // MD_MOD_H
