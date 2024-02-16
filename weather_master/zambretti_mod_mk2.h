// made referencing this:
// https://github.com/sassoftware/iot-zambretti-weather-forcasting
#ifndef ZAMBRETTI_MOD_MK2_H
#define ZAMBRETTI_MOD_MK2_H
#include "HardwareSerial.h"
#include "st.h"
#include <cstdint>
#include <cstdio>
#define ZAMBRETTI_SAMPLE 10
// includes
#include <math.h>
#include "clock_mod.h"
#include "temp_mod.h"
#include "storage_mod.h"
#include "sl_device_mod.h" // there shouldnt be a difference between the
                           // pressure but might as well
                           // interface
                           // variables
#define SECONDS_IN_A_MINUTE 60
#define TIME_BTW_MEASURMENTS (18*SECONDS_IN_A_MINUTE * 1000)
static modState ZAMBRETTI_state = ModUninitialized; // module state
static int sleep_interval = TIME_BTW_MEASURMENTS; // module state
static float pressureArray[ZAMBRETTI_SAMPLE]={0};
#define ZAMBRETTI_BACK_SIZE 256
#define ZAM_BACKUPNAME "/.zam.bak"
static char ZAM_buffer[ZAMBRETTI_BACK_SIZE] = {0};
static int last_hour = 0;
static int last_minute = 0;
static int delta_time = 0;
static int max_delta = 10;
static int counter = 0;
static std::mutex zambretti_mutex; // since this runs in a separate trhead a
                                   //
int32_t log2(int32_t num ){
    int l = 0;
    while (num >>= 1) { ++l; }
    return l;
}
// effectively turns it into a builder  + bit field
//
//
//
enum Pressure {
    Steady             = 0,
    Falling            = 1 << 0,
    Rising             = 1 << 1,
};

enum WeatherNow {
    Changable      = Rising << 1,
    Intermittent   = Rising << 2,
    Sunny          = Rising << 3,
    Rain           = Rising << 4,
    Stormy         = Rising << 5,
    Showery        = Rising << 6,
    Fine           = Rising << 7,
    Cloudy         = Rising << 8,
};
enum WeatherLater {
    Maybe              = Cloudy << 1,
    Intermittent_Later = Cloudy << 2,
    Sunny_Later        = Cloudy << 3,
    Rain_Later         = Cloudy << 4,
    Stormy_Later       = Cloudy << 5,
    Showery_Later      = Cloudy << 6,
    Fine_Later         = Cloudy << 7,
    Cloudy_Later       = Cloudy << 8,
};

enum UnSettled {
    Becoming           = Cloudy_Later << 5,
    Improving          = Cloudy_Later << 6,
    Settled            = Cloudy_Later << 7,
    Slightly           = Cloudy_Later << 8,
    Unsettled          = Cloudy_Later << 9,
    More               = Cloudy_Later << 10,
    Very               = Cloudy_Later << 11,
    Worse              = Cloudy_Later << 12,
    CANNOT_MAKE        = Cloudy_Later << 13,
};

String printables[] = {
    "Steady",
    "Falling",
    "Rising",
    "Changable",
    "Intermittent",
    "Sunny",
    "Rain",
    "Stormy",
    "Showery",
    "Fine",
    "Cloudy",
    "Maybe",
    "Interm Later",
    "Sunny Later",
    "Rain Later",
    "Stormy Later",
    "Showery Later",
    "Fine Later",
    "Cloudy Later",
    "Becoming",
    "Improving",
    "Settled",
    "Slightly",
    "Unsettled",
    "More",
    "Very",
    "Worse",
    "CANNOT MAKE",
};

//weather lookup table
const int32_t zambretti_table[]
{
        CANNOT_MAKE,
        Settled      |  Fine,        // 1
        Fine,        // 2
        Fine         |  Becoming     |  Slightly,          // 3
        Fine         |  Rain_Later,  // 4
        Rain         |  Becoming     |  More,              // 5
        Unsettled    |  Rain_Later,  // 6
        Rain         |  Becoming     |  Worse,             // 7
        Rain         |  Becoming     |  Very,              // 8
        Very         |  Rain_Later,  // 9
        Settled      |  Fine,        // 10
        Fine,        // 11
        Fine         |  Maybe        |  Showery_Later,     // 12
        Fine         |  Maybe        |  Showery_Later,     // 13
        Showery      |  Maybe        |  Intermittent_Later |  Sunny_Later, // 14
        Changable    |  Maybe        |  Rain_Later,        // 15
        Unsettled    |  Maybe        |  Rain_Later,        // 16
        Intermittent |  Rain,        // 17
        Very         |  Rain,        // 18
        Stormy,      // 19
        Settled      |  Fine,        // 20
        Fine,        // 21
        Fine_Later,  // 22
        Fine         |  Improving,   // 23
        Fine         |  Maybe        |  Showery_Later,     // 24
        Showery      |  Improving,   // 25
        Changable    |  Improving,   // 26
        Unsettled    |  Improving,   // 27
        Unsettled    |  Maybe        |  Improving,         // 28
        Unsettled    |  Intermittent |  Fine,              // 29
        Very         |  Intermittent |  Fine,              // 30
        Stormy       |  Maybe        |  Improving,         // 31
        Stormy       |  Showery,     // 32
};

static int32_t Forecast = CANNOT_MAKE;
static int weather_state = 0;

// functuions
float station2sealevel(float p, float height, float t){
    return  p*pow(1-0.0065*(float)height/(t+0.0065*(float)height+273.15),-5.275);
}

inline int last_idx(){
    int i = 0 ;
    while( i < ZAMBRETTI_SAMPLE && pressureArray[i]){
        i++;
    }
    return i;
}


inline Pressure is_falling (float Cprs, float Pprs) {
    if (( Cprs<= 1050 ) && ( Pprs <= 1050 ) && (Cprs- Pprs > 1.6)) {
        return Falling;
    }
    return Steady;
}

inline Pressure is_rising (float Cprs, float Pprs){
    if (( Cprs<= 1030 ) && ( Pprs <= 1050 ) && (Pprs - Cprs> 1.6)){
        return Rising;
    }
    return Steady;
}

int calc_zambretti_index(float curr_pressure, float prev_pressure) {
    // should be Pressure but enums
    int p = is_falling(curr_pressure, prev_pressure) | is_rising(curr_pressure, prev_pressure); 
    // linear interpolation
    switch (p) {
        case Steady:
            return 144.0 - 0.13 * curr_pressure;
        case Falling:
            return 127.0 - 0.12 * curr_pressure;
        case Rising:
            return 185.0 - 0.16 * curr_pressure;
    }
}

int32_t calc_zambretti(float curr_pressure, float prev_pressure) {
    int zambretti_idx = calc_zambretti_index(curr_pressure, prev_pressure);
    int pressure_trend = is_falling(curr_pressure, prev_pressure) |
                         is_rising(curr_pressure, prev_pressure);
    if (1 <= zambretti_idx && zambretti_idx <= sizeof(zambretti_table) / sizeof(int)) {
        Serial.println(zambretti_idx);
        return zambretti_table[zambretti_idx];
    } else {
        return 0;
    }
}

extern struct_message data_state;

extern float TEMP_temp;
extern float TEMP_humd;
extern float TEMP_pres;
extern float TEMP_altd;

void ZAMBRETTI_backup(){
    memset(ZAM_buffer, 0, ZAMBRETTI_BACK_SIZE);
    sprintf(ZAM_buffer,
            "%f %f %f %f %f %f %f %f %f %f" ,
            pressureArray[0],
            pressureArray[1],
            pressureArray[2],
            pressureArray[3],
            pressureArray[4],
            pressureArray[5],
            pressureArray[6],
            pressureArray[7],
            pressureArray[8],
            pressureArray[9]);
    writeFile(SD, ZAM_BACKUPNAME, ZAM_buffer);
};

void ZAMBRETTI_load_backup(){
    memset(ZAM_buffer, 0, ZAMBRETTI_BACK_SIZE);
    if(readFileToBuf(SD,ZAM_BACKUPNAME, ZAM_buffer, ZAMBRETTI_BACK_SIZE)){
        for (int i=0; i<9;i++) {
            pressureArray [i] =0;}
        return ;
    }
    int offs = 0;
    int num = 0;
    sscanf(ZAM_buffer,
            "%f %f %f %f %f %f %f %f %f %f" ,
            &pressureArray[0],
            &pressureArray[1],
            &pressureArray[2],
            &pressureArray[3],
            &pressureArray[4],
            &pressureArray[5],
            &pressureArray[6],
            &pressureArray[7],
            &pressureArray[8],
            &pressureArray[9]);
   return;
};

int32_t zambretti_weather() { // retruns a bit field of weather
    float temperature = data_state.temp;
    float humidity=data_state.hum;
    float pressure=data_state.pres;

    // set default values to these if there are no arriving values from outdoor
    // device
    temperature = temperature == 0.0 ? TEMP_temp : temperature;
    humidity = humidity == 0.0 ? TEMP_humd : humidity;
    pressure = pressure ==0.0 ?  TEMP_pres : pressure;

    int altitude=TEMP_altd;
    int seapressure = station2sealevel(pressure,altitude,temperature);
    int idx = last_idx(); 
    if (idx == 10) {
        for (int i=0; i<9;i++) {
            pressureArray[i]=pressureArray[i+1];
        }
        pressureArray[counter-1]=seapressure; 
    } else {
        pressureArray[idx]=seapressure;
    }

    zambretti_mutex.lock();
    Forecast = calc_zambretti((pressureArray[9]+pressureArray[8]+pressureArray[7])/3,(pressureArray[0]+pressureArray[1]+pressureArray[2])/3);
    Serial.println(Forecast);
    ZAMBRETTI_backup();
    zambretti_mutex.unlock();
    return Forecast;
}

void ZAMBRETTI_update_async(void * parameter){
    while(1){
        zambretti_weather();
        delay(TIME_BTW_MEASURMENTS);
    }
}


#define ZAMBRETTI_TASK_CREATE xTaskCreate( ZAMBRETTI_update_async, "Handle Updating of the weather forecast", 4096, NULL, 1, NULL);
modState ZAMBRETTI_init() {
    if(ZAMBRETTI_state) {
        ZAMBRETTI_load_backup();
        ZAMBRETTI_TASK_CREATE;
    }
    ZAMBRETTI_state = ModOK;
    return ZAMBRETTI_state;
}

#endif // ZAMBRETTI_MOD_H
