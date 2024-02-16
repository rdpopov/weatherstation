/**
 * @file network_mod.h
 * @brief  Module to handle connection to wireless AP or create one own
 */

#ifndef NETWORK_MOD_H
#define NETWORK_MOD_H
#include "st.h"
#include "config_mod.h"
#include <WiFiProv.h>

// includes
// interface
// variables
static modState NETWORK_state = ModUninitialized; // module state
                                                  //
static const char * ssid = NULL;
static const char * password = NULL;
extern bool softAP = false;
extern DynamicJsonDocument json_conf;

// functuions

// ------------------------
// base interface 
// ------------------------


/**
 * @brief Connect to AP specified in config.
 * Otherwise wait 15 seconds and create a backup one
 *
 * @return Module state
 */
modState NETWORK_init() { 
    if (NETWORK_state) {
        ssid = json_conf["ssid"] ;
        password = json_conf["pass"] ;

        const char * def_ssid = json_conf["dssid"] ;
        const char * def_password = json_conf["dpass"] ;

        Serial.print("Connecting to ");
        Serial.println(ssid);
        Serial.println(password);
        int8_t tries = 30;
        WiFi.mode(WIFI_AP_STA);
        WiFi.begin(ssid, password);
        while (WiFi.status() != WL_CONNECTED && tries--) {
            delay(500);
            Serial.print(".");
        }
        if (WiFi.status() != WL_CONNECTED){
            Serial.println("Have to create network");
            ssid = def_ssid;
            Serial.println(def_ssid);
            Serial.println(def_password);
            WiFi.softAP(def_ssid, def_password);
            Serial.println(WiFi.softAPIP());
            softAP = true;
        } else {
            Serial.println("WiFi connected.");
            Serial.println("IP address: ");
            Serial.println(WiFi.localIP());
        }
    }
    return NETWORK_state;
}

#endif // MD_MOD_H
