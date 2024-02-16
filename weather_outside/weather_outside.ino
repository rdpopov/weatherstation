/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

const char* ssid = "Rosko";
const char* backup_ssid = "E32Wtr-St";
static Adafruit_BME280 bme;
#define SEND_RETRIES 5

uint8_t broadcastAddress[] = {0x98, 0xF4, 0xAB, 0x00, 0xF9, 0x00};
String success;

typedef struct struct_message {
    float temp;
    float hum;
    float pres;
    float uv_idx;
} struct_message;

struct_message EnvUv;

esp_now_peer_info_t peerInfo;

int32_t getWiFiChannel(const char *ssid) {
  if (int32_t n = WiFi.scanNetworks()) {
      for (uint8_t i=0; i<n; i++) {
          if (!strcmp(ssid, WiFi.SSID(i).c_str())) {
              return WiFi.channel(i);
          }
      }
  }
  return 0;
}
// we make the device enter in a loop to find a network so it can update
// if once the info cant be sent it attempts again
int32_t connect_wifi(){

    int32_t channel = getWiFiChannel(ssid);
    int32_t tries = 10;
    while (tries--) {
        if (channel = 0) { // main ap with name ssid doesnt exist, therefore we knwo
                           // that the other one will create a backup one to host ... things
            channel = getWiFiChannel(backup_ssid);
        } else {
            break;
        }
        delay(1000); // assuming both are powered up at once, the other devcie
                     // maybe has to timeout until it creates an acess point
    }

    WiFi.printDiag(Serial);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Failed to add peer");
        return -1;
    }
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = channel;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return -2;
    }

    esp_now_register_send_cb(OnDataSent);
    return 0;
}

void setup() {
    Serial.begin(9600);

    bool status = bme.begin(0x77);
    if (!status) {
        Serial.println("Could not find a valid BME280 sensor, check wiring!");
        while (1);
    }
    WiFi.mode(WIFI_STA);
    connect_wifi();
}

void loop() {
    EnvUv.temp =  bme.readTemperature();
    EnvUv.hum =  bme.readHumidity();
    EnvUv.pres =  bme.readPressure() / 100.0F; 
    int sensorValue;
    sensorValue=analogRead(14);
    float sensorVoltage = (sensorValue * 3.3)  / 1024 ;
    float UV_index_float = sensorVoltage / 0.1;
    EnvUv.uv_idx = UV_index_float;
    int32_t  retr = SEND_RETRIES;
    esp_err_t result = 1;
    do {
        result = esp_now_send(broadcastAddress, (uint8_t *) &EnvUv, sizeof(EnvUv));
    } while (result != ESP_OK && --retr );

    Serial.printf("%02f %02f %02f %02f res: %d \n", EnvUv.temp, EnvUv.hum ,
            EnvUv.pres, UV_index_float,result);

    if (result == ESP_OK ) {
        delay(300000);
    } else {
        connect_wifi();
    }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
