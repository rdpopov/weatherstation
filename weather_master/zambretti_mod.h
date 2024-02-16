#ifndef ZAMBRETTI_MOD_H
#define ZAMBRETTI_MOD_H
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
static modState ZAMBRETTI_state = ModUninitialized; // module state
static int sleep_interval = 18 * 60 * 1000 // module state
static int pressureArray[ZAMBRETTI_SAMPLE]={0};
#define ZAMBRETTI_BACK_SIZE 256
#define ZAM_BACKUPNAME "/.zam.bak"
static char ZAM_buffer[ZAMBRETTI_BACK_SIZE] = {0};
static int last_hour = 0;
static int last_minute = 0;
static int delta_time = 0;
static int max_delta = 10;
static int counter = 0;
static std::mutex zambretti_mutex; // since this runs in a separate trhead a
                                   // mutex is in order
// effectively turns it into a builder
enum WeatherState {
    Sunny     = 1 << 0,
    Rainy     = 1 << 1,
    Cloudy    = 1 << 2,
    Worsening = 1 << 3,
    Steady    = 1 << 4,
    Falling   = 1 << 5,
    Rising    = 1 << 6,
};

static int weather_state = 0;

// functuions
int station2sealevel(int p, int height, int t){
    return (double) p*pow(1-0.0065*(double)height/(t+0.0065*(double)height+273.15),-5.275);
}
inline int last_idx(){
    int i = 0 ;
    while( i < ZAMBRETTI_SAMPLE && pressureArray[i]){
        i++;
    }
    return i;
}

int calc_zambretti(int curr_pressure, int prev_pressure, int mon) {
    Serial.printf(" attempt calc %d %d %d", curr_pressure, prev_pressure, mon);
    if (curr_pressure<prev_pressure) { //FALLING
        if (mon>=4 && mon<=9) { //summer
            if (curr_pressure>=1030) return 2;
            else if(curr_pressure>=1020 && curr_pressure<1030) return 8;
            else if(curr_pressure>=1010 && curr_pressure<1020) return 18;
            else if(curr_pressure>=1000 && curr_pressure<1010) return 21;
            else if(curr_pressure>=990 && curr_pressure<1000) return 24;
            else if(curr_pressure>=980 && curr_pressure<990) return 24;
            else if(curr_pressure>=970 && curr_pressure<980) return 26;
            else if(curr_pressure<970) return 26;
        } else { //winter
            if (curr_pressure>=1030) return 2;
            else if(curr_pressure>=1020 && curr_pressure<1030) return 8;
            else if(curr_pressure>=1010 && curr_pressure<1020) return 15;
            else if(curr_pressure>=1000 && curr_pressure<1010) return 21;
            else if(curr_pressure>=990 && curr_pressure<1000) return 22;
            else if(curr_pressure>=980 && curr_pressure<990) return 24;
            else if(curr_pressure>=970 && curr_pressure<980) return 26;
            else if(curr_pressure<970) return 26;
        }
    }
    else if (curr_pressure>prev_pressure) { //RAISING
        if (mon>=4 && mon<=9) { //summer
            if (curr_pressure>=1030) return 1;
            else if(curr_pressure>=1020 && curr_pressure<1030) return 2;
            else if(curr_pressure>=1010 && curr_pressure<1020) return 3;
            else if(curr_pressure>=1000 && curr_pressure<1010) return 7;
            else if(curr_pressure>=990 && curr_pressure<1000) return 9;
            else if(curr_pressure>=980 && curr_pressure<990) return 12;
            else if(curr_pressure>=970 && curr_pressure<980) return 17;
            else if(curr_pressure<970) return 17;
        } else { //winter
            if (curr_pressure>=1030) return 1;
            else if(curr_pressure>=1020 && curr_pressure<1030) return 2;
            else if(curr_pressure>=1010 && curr_pressure<1020) return 6;
            else if(curr_pressure>=1000 && curr_pressure<1010) return 7;
            else if(curr_pressure>=990 && curr_pressure<1000) return 10;
            else if(curr_pressure>=980 && curr_pressure<990) return 13;
            else if(curr_pressure>=970 && curr_pressure<980) return 17;
            else if(curr_pressure<970) return 17;
        }
    } else {
        if (curr_pressure>=1030) return 1;
        else if(curr_pressure>=1020 && curr_pressure<1030) return 2;
        else if(curr_pressure>=1010 && curr_pressure<1020) return 11;
        else if(curr_pressure>=1000 && curr_pressure<1010) return 14;
        else if(curr_pressure>=990 && curr_pressure<1000) return 19;
        else if(curr_pressure>=980 && curr_pressure<990) return 23;
        else if(curr_pressure>=970 && curr_pressure<980) return 24;
        else if(curr_pressure<970) return 26;
    }
    return 0;
}

extern struct_message data_state;

extern float TEMP_temp;
extern float TEMP_humd;
extern float TEMP_pres;
extern float TEMP_altd;

void ZAMBRETTI_backup(){
    memset(ZAM_buffer, 0, ZAMBRETTI_BACK_SIZE);
    sprintf(ZAM_buffer,
            "%d %d %d %d %d %d %d %d %d %d" ,
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
            "%d %d %d %d %d %d %d %d %d %d" ,
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
    int temperature = data_state.temp;
    int humidity=data_state.hum;
    int pressure=data_state.pres;

    // set default values to these if there are no arriving values from outdoor
    // device
    temperature = temperature == 0.0 ? TEMP_temp : temperature;
    humidity = humidity == 0.0 ? TEMP_humd : humidity;
    pressure = pressure ==0.0 ?  TEMP_pres : pressure;

    int altitude=TEMP_altd;
   int seapressure = station2sealevel(pressure,altitude,temperature);
    int crnt_hour=CurrentTime.hour();
    int crnt_minute=CurrentTime.minute();
    int Z=0;
    if (crnt_hour!=last_hour || crnt_minute!=last_minute && pressure > 0 && seapressure > 0) {
        zambretti_mutex.lock();
        weather_state = 0;
        delta_time++;
        if (delta_time>max_delta){
            delta_time=0;
            int idx = last_idx(); 
            if (idx == 10) {
                for (int i=0; i<9;i++) {
                    pressureArray[i]=pressureArray[i+1];
                }
                pressureArray[counter-1]=seapressure; 
            } else {
                pressureArray[idx]=seapressure;
            }
            Serial.println("BACKUP");
            ZAMBRETTI_backup();
        }
        Z = calc_zambretti((pressureArray[9]+pressureArray[8]+pressureArray[7])/3,(pressureArray[0]+pressureArray[1]+pressureArray[2])/3, CurrentTime.month());
        Serial.println("Forecast ");
        Serial.println(Z);
        if (pressureArray[9]>0 && pressureArray[0]>0) {
            if (pressureArray[9]+pressureArray[8]+pressureArray[7]-pressureArray[0]-pressureArray[1]-pressureArray[2]>=3){
                //RAISING
                weather_state |= Rising;
                if ( Z < 3 ) weather_state |= Sunny;
                else if ( 3 <= Z && Z <= 9 ) weather_state |= Sunny | Cloudy;
                else if ( 9 < Z && Z <=17 ) weather_state |=  Cloudy;
                else if ( 17 < Z) weather_state |= Cloudy;
            }
            else if (pressureArray[0]+pressureArray[1]+pressureArray[2]-pressureArray[9]-pressureArray[8]-pressureArray[7]>=3){
                //FALLING
                weather_state |= Falling;
                if (Z<4) weather_state |= Sunny;
                else if (Z>=4 && Z<14) weather_state |= Sunny | Cloudy;
                else if (Z>=14 && Z<19) weather_state |= Worsening;
                else if (Z>=19 && Z<21) weather_state |= Cloudy;
                else if (21<=Z) weather_state |= Rainy;
            }
            else{
                //STEADY
                weather_state |= Steady;
                if ( Z<5 ) weather_state |= Sunny;
                else if ( 5<=Z && Z<=11 ) weather_state |= Sunny| Cloudy;
                else if (11 < Z && Z < 14) weather_state |= Cloudy;
                else if (14<=Z && Z<19) weather_state |= Worsening;
                else if (19<Z) weather_state |= Rainy;
            }
        } else {
            if (seapressure<1005) weather_state |= Rainy;
            else if (1005<=seapressure && seapressure<=1015) weather_state |= Cloudy;
            else if (1015 < seapressure && seapressure<1025) weather_state |= Sunny | Cloudy;
            else weather_state |= Rainy;
        }

        last_hour = crnt_hour;
        last_minute = crnt_minute;
        zambretti_mutex.unlock();
    }
    return weather_state;
}
// ------------------------
// base interface
// ------------------------

void ZAMBRETTI_update_async(void * parameter){
    while(1){
        zambretti_weather();
        delay(sleep_interval);
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

modState ZAMBRETTI_update(){
    return ModOK;
}

int ZAMBRETTI_get(){
    return 0;
}

modState ZAMBRETTI_log(){ 
#ifdef LOG_MOD_H
    // do the log part
#endif
    return ModOK;
}

#endif // ZAMBRETTI_MOD_H
