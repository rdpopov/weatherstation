#ifndef MD_MOD_H
#define DISPLAY_MOD_H
#include "DS3231.h"
#include "astro_mod.h"
#include "esp32-hal.h"
#include "esp_heap_caps.h"
#include "gas_mod.h"
#include "network_mod.h"
#include "temp_mod.h"
#include "clock_mod.h"
#include "astro_mod.h"
#include "sl_device_mod.h"
#include "zambretti_meta_mod.h"
#include "config_mod.h"
#include "st.h"
// includes
#include <Wire.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED width,  in pixels
#define SCREEN_HEIGHT 64 // OLED height, in pixels

#define CLEAN_PRINT_WAIT( BCLEAN, CALL, TIMEOUT) do { \
    if (BCLEAN) oled.clearDisplay(); \
    CALL(); \
    oled.display(); \
    delay(TIMEOUT); \
} while(0) \

// create an OLED display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
static modState DISPLAY_state = ModUninitialized; // module state

/**
 * @brief Init display module
 *
 * @return module state
 */
modState DISPLAY_init() {
    if (DISPLAY_state) {
        if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Serial.println("failed to start SSD1306 OLED");
            DISPLAY_state = ModError;
        }

        int pos = 0;
        delay(2000);         // wait two seconds for initializing
        oled.clearDisplay(); // clear display
        oled.setTextSize(1);         // set text size
        oled.setTextColor(WHITE);    // set text color
        oled.setCursor(0,pos);       // set position to display
        oled.println("Booting"); // set text
        if (CONFIG_state){
            pos =+ 8;
            oled.setCursor(0, pos);       // set position to display
            oled.println("Wrong config"); // set text
        }
        if (softAP){
            pos =+ 8;
            oled.setCursor(0, pos);       // set position to display
            oled.println("Using backup AP"); // set text
        }
        oled.display();
        DISPLAY_state = ModOK;
    }
    return DISPLAY_state;
}

/**
 * @brief Update method for dislpay
 *
 * @return module state
 */
modState DISPLAY_update(){ 
    return ModOK;
}

#ifdef GAS_MOD_H
extern float GAS_ppm;
/**
 * @brief Displays information from the GAS module
 *
 * @return module state
 */
modState GAS_display(){ 
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 10);       // set position to display
    oled.printf("CO ppm:%f",GAS_ppm);
    /* Serial.printf("CO ppm:%f,",GAS_ppm); */
    return ModOK;
}
#endif // GAS_MOD_H

#ifdef TEMP_MOD_H
extern float TEMP_temp;
extern float TEMP_pres;
extern float TEMP_altd;
extern float TEMP_humd;

/**
 * @brief Displays information from the TEMP module
 *
 * @return module state
 */
modState TEMP_display(){ 
    /* TEMP_update(); */
    int ypos =0;
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Inside");
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Temp: %.2f C", TEMP_temp);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Pres: %.2f hpa", TEMP_pres);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Altd: %.2f m", TEMP_altd);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Humd: %.2f %%", TEMP_humd);
    /* Serial.printf("Temperature:%f C, Pressure:%f HPA, Altitude:%f m, Humidity:%f,", */
    /*         TEMP_temp, TEMP_pres, TEMP_altd, TEMP_humd); */
    return TEMP_state;
}
#endif // TEMP_MOD_H

#ifdef OUTSIDE_RECIEVER_MOD_H
extern struct_message data_state;
/**
 * @brief Displays information from the OUTSIDE_RECIEVER module
 *
 * @return module state
 */
modState OUTSIDE_RECIEVER_display(){
    int ypos =0 ;
    OUTSIDE_RECIEVER_mutex.lock();
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Outside");
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Temp: %.2f C", data_state.temp);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Pres: %.2f hpa", data_state.pres);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Humd: %.2f %%", data_state.hum);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("UV: %.2f ", data_state.uv_idx);
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    /* oled.printf("Battery: %.2f ", data_state.batt_prc); */
    OUTSIDE_RECIEVER_mutex.unlock();
    return TEMP_state;
}
#endif // OUTSIDE_RECIEVER_MOD_H

#ifdef CLOCK_MOD_H
extern DateTime CurrentTime;
extern const char * ssid;

/**
 * @brief Displays information from the CLOCK module
 *
 * @return module state
 */
modState CLOCK_display(){
    /* CLOCK_update(); */
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 8);        // set position to display
    oled.printf("Date: %d/%02d/%02d ", CurrentTime.day(),CurrentTime.month(),CurrentTime.year());
    oled.setCursor(0, 16);        // set position to display
    oled.printf("Time: %02d:%02d", CurrentTime.hour(),CurrentTime.minute());
    oled.setCursor(0, 24);        // set position to display
    oled.print("AP: ");
    oled.print(ssid);
    oled.setCursor(0, 32);        // set position to display
    oled.print("IP: ");
    oled.print(WiFi.localIP());
    /* Serial.println(timeClient.getFormattedTime()); */
    /* Serial.printf("Date: %d/%d/%d %d:%d:%d\n" ,CurrentTime.day(),CurrentTime.month(),CurrentTime.year(),CurrentTime.hour(),CurrentTime.minute(),CurrentTime.second()); */
    return ModOK;
}
#endif // CLOCK_MOD_H

#ifdef ZAMBRETTI_MOD_H
extern int weather_state;
extern std::mutex zambretti_mutex;

/**
 * @brief Displays information from the ZAMBRETTI module
 *
 * @return module state
 */

modState ZAMBRETTI_display(){
    zambretti_mutex.lock();
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, 8);        // set position to display
    oled.println("Forecast");
    oled.setCursor(0, 16);        // set position to display
    if(weather_state & Steady) oled.println("Steady ");
    if(weather_state & Falling) oled.println("Falling ");
    if(weather_state & Rising) oled.println("Rising ");
    oled.setCursor(0, 24);
    if(weather_state & Worsening) oled.println("Worsening ");
    if(weather_state & Rainy) oled.println("Rainy ");
    if(weather_state & Cloudy) oled.println("Cloudy ");
    if(weather_state & Sunny) oled.println("Sunny ");
    zambretti_mutex.unlock();
    return ModOK;
}
#endif // ZAMBRETTI_MOD_H


#ifdef ZAMBRETTI_MOD_MK2_H
extern int32_t Forecast;
extern std::mutex zambretti_mutex;


/**
 * @brief Displays information from the ZAMBRETTI module
 *
 * @return module state
 */

modState ZAMBRETTI_display(){
    int ypos = 0;
    zambretti_mutex.lock();
    int32_t lF = Forecast;
    zambretti_mutex.unlock();
    oled.setCursor(0,0);        // set position to display
    oled.println("Forecast");
    oled.setCursor(0, ypos+=8);        // set position to display
    oled.print(printables[lF & 0x3]); // either 1 2 or 0
    oled.println(" Pressure");
    oled.setCursor(0, ypos+=8);        // set position to display
    int sz = log2(lF);
    for (int i = 2; i <= sz; i++ ){
        if ((1 << i) & lF) {
            oled.println(printables[i+1]);
            oled.setCursor(0, ypos+=8);        // set position to displ
        }
    }
    return ModOK;
}
#endif // ZAMBRETTI_MOD_H

#ifdef ASTRO_MOD_H
extern moonData_t moon_dat;
extern MoonRise moon_rise;
extern DateTime SRTime;
extern DateTime SSTime;
extern DateTime MRTime;
extern DateTime MSTime;

/**
 * @brief Displays information from the ASTRO module
 *
 * @return module state
 */
modState ASTRO_display(){
    /* ASTRO_update(); */
    ASTRO_mutex.lock();
    int ypos = 0;
    oled.setTextSize(1);         // set text size
    oled.setTextColor(WHITE);    // set text color
    oled.setCursor(0, ypos);
    oled.printf("Sun:");
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Sun Rise: %02d:%02d" ,(SRTime.hour() + gmt + ds) % 24 ,SRTime.minute());
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Sun Set: %02d:%02d" ,(SSTime.hour() + gmt + ds) % 24,SSTime.minute());
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Moon Rise: %02d:%02d" ,(MRTime.hour() + gmt + ds) % 24,MRTime.minute());
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Moon Set: %02d:%02d" ,(MSTime.hour() + gmt + ds) % 24,MSTime.minute());
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    oled.printf("Ang: %d o Lit: %.2f", moon_dat.angle,moon_dat.percentLit * 100  );
    oled.setCursor(0, ypos); ypos +=8;       // set position to display
    ASTRO_mutex.unlock();
    return ModOK;
}
#endif // ASTRO_MOD_H

/**
 * @brief Display module basic loop
 *
 * @return Module state
 */
modState DISPLAY_display(){
    CLEAN_PRINT_WAIT(true,GAS_display,5000);
    CLEAN_PRINT_WAIT(true,TEMP_display,5000);
    CLEAN_PRINT_WAIT(true,OUTSIDE_RECIEVER_display,5000);
    /* CLEAN_PRINT_WAIT(true,ZAMBRETTI_display,5000); */
    CLEAN_PRINT_WAIT(true,ZAMBRETTI_display,5000);
    CLEAN_PRINT_WAIT(true,ASTRO_display,5000);
    CLEAN_PRINT_WAIT(true,CLOCK_display,5000);
    Serial.printf("DISPLAY:%d,",0);
    return ModOK;
}


/**
 * @brief Thread function for display
 *
 * @param parameter (unused)
 */
void DISPLAY_display_inf(void * parameter){
    while(1){
        DISPLAY_display();
    }
}
#define DISPLAY_TASK_CREATE xTaskCreate( DISPLAY_display_inf, "Handle Display Repfresh", 2048 + 4096, NULL, 1, NULL);

#endif // DISPLAY_MOD_H
