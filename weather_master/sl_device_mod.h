/**
 * @file sl_device_mod.h
 * @brief file that aims to get the value from the slave device
 */

#ifndef OUTSIDE_RECIEVER_MOD_H
#define OUTSIDE_RECIEVER_MOD_H
#include "st.h"
#include <esp_now.h>
// includes
typedef struct struct_message {
    float temp;
    float hum;
    float pres;
    float uv_idx;
    float batt_prc;
} struct_message;
static struct_message indReadings;
static struct_message data_state;
static std::mutex OUTSIDE_RECIEVER_mutex;

/**
 * @brief on data recieve callback
 *
 * @param mac mac addr
 * @param indData raw data
 * @param len length of recieved data
 */
void OnDataRecv(const uint8_t * mac, const uint8_t *indData, int len) {
    OUTSIDE_RECIEVER_mutex.lock();
    memcpy(&indReadings, indData, sizeof(indReadings));
    Serial.print("Bytes received: ");
    Serial.println(len);

    data_state.temp     = indReadings.temp;
    data_state.hum      = indReadings.hum;
    data_state.pres     = indReadings.pres;
    data_state.uv_idx   = indReadings.uv_idx;
    data_state.batt_prc = indReadings.batt_prc;
    OUTSIDE_RECIEVER_mutex.unlock();
}
// variables
static modState OUTSIDE_RECIEVER_state = ModUninitialized; // module state

esp_now_peer_info_t peerInfo;
// functuions

// ------------------------
// base interface 
// ------------------------

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/**
 * @brief Init the esp now communication
 *
 * @return [TODO:return]
 */
modState OUTSIDE_RECIEVER_init() { 
    if (OUTSIDE_RECIEVER_state){
        if (esp_now_init() != ESP_OK) {
            OUTSIDE_RECIEVER_state = ModError;
            Serial.println("Error initializing ESP-NOW");
            return OUTSIDE_RECIEVER_state;
        }
        esp_now_register_recv_cb(OnDataRecv);
        OUTSIDE_RECIEVER_state = ModOK;
    }
    return OUTSIDE_RECIEVER_state;
}

/**
 * @brief Log current data to file
 *
 * @return [TODO:return]
 */
modState OUTSIDE_RECIEVER_log(){ 
#ifdef LOG_MOD_H
    OUTSIDE_RECIEVER_mutex.lock();
    LOG_write_float("TempOutside",data_state.temp);
    LOG_write_float("HumdOutside",data_state.hum);
    LOG_write_float("PresOutside",data_state.pres);
    LOG_write_float("UvIdx",data_state.uv_idx);
    LOG_write_float("BatteryPerc",data_state.batt_prc);
    OUTSIDE_RECIEVER_mutex.unlock();
#endif
    return ModOK;
}

#endif // MD_MOD_H
