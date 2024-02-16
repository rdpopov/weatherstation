//includes
// ============================================
// system critical modules ====================
#include "storage_mod.h" // sd card
#include "config_mod.h" // configuration module
#include "network_mod.h" // configuration module
#include "clock_mod.h"   // clock
// ============================================

// ============================================
// sensor modules =============================
#include "temp_mod.h" // temp sensor
#include "gas_mod.h"  // gas sensor
// meta sensor modules ========================
#include "sl_device_mod.h" // outside device module
#include "astro_mod.h" // astro module
#include "zambretti_meta_mod.h" // astro module
// ============================================

// ============================================
// functional modules =========================
#include "logger_mod.h"  // logging module
#include "display_mod.h" // display module
#include "websrv_mod.h" // web server module
// ============================================

extern int log_delay;

void setup() {
    Serial.begin(9600);
    Serial.println(WiFi.macAddress());
    SD_STORAGE_init();
    CONFIG_init();
    NETWORK_init();
    DISPLAY_init();
    CLOCK_init();
    TEMP_init();
    WEBSRV_init();
    ASTRO_init();
    OUTSIDE_RECIEVER_init();
    GAS_init();
    LOG_init();
    ZAMBRETTI_init(); // creates a task to update forecast
    DISPLAY_TASK_CREATE;
}

void loop(){
    TEMP_update();
    GAS_update();
    ASTRO_update();

    // log every n seconds
    static int compound_delay = 0;
    if(compound_delay > log_delay) {
        OUTSIDE_RECIEVER_log();
        GAS_log();
        TEMP_log();
        LOG_log();
        compound_delay %= log_delay;
    } 
    compound_delay +=5000;
    delay(5000); // second delay
} 


