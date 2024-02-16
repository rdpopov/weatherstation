#ifndef TEMP_MOD_H
#define TEMP_MOD_H
#include "logger_mod.h"
#include "st.h"
// includes
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)

// interface
// variables
static modState TEMP_state = ModUninitialized; // module state
static Adafruit_BME280 bme; // I2C
static float TEMP_temp;
static float TEMP_pres;
static float TEMP_altd;
static float TEMP_humd;
static std::mutex TEMP_mutex;
// functuions
// ------------------------
// base interface 
// ------------------------

/**
 * @brief Initializes BME280 sensor
 *
 * @return Module state
 */
modState TEMP_init() { 
    bool status;
    if (TEMP_state) {
        status = bme.begin(0x77);
        if (!status) {
            TEMP_state = ModError;
            Serial.println("Could not find a valid BME280 sensor, check wiring!");
        }
    }
    TEMP_state = ModOK;
    return TEMP_state;
}

/**
 * @brief Cache information from the sensor
 *
 * @return Module state
 */
modState TEMP_update(){ 
    TEMP_mutex.lock();
    if(TEMP_state) {
        TEMP_init();
    }
    if(!TEMP_state){
        TEMP_temp = bme.readTemperature(); 
        TEMP_pres = bme.readPressure() / 100.0F; 
        TEMP_altd = bme.readAltitude(SEALEVELPRESSURE_HPA); 
        TEMP_humd = bme.readHumidity(); 
    }
    TEMP_mutex.unlock();
    return TEMP_state;
}

/**
 * @brief Log information to log file 
 *
 * @return Module state
 */
modState TEMP_log(){ 
#ifdef LOG_MOD_H
    LOG_write_float("TempInside", TEMP_temp);
    LOG_write_float("PresInside", TEMP_pres);
    LOG_write_float("AltdInside", TEMP_altd);
    LOG_write_float("HumdInside", TEMP_humd);
#endif
    return TEMP_state;
}

#endif // MD_MOD_H
