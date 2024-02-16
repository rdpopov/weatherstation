#ifndef WEBSRV_MOD_H
#define WEBSRV_MOD_H
#include "clock_mod.h"
#include "network_mod.h"
#include "st.h"
#include "config_mod.h"

#include "astro_mod.h"
#include "gas_mod.h"
#include "sl_device_mod.h"
#include "temp_mod.h"
#include "zambretti_meta_mod.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <cstdio>
#include <cstring>
// includes
// interface
// variables
static modState WEBSRV_state = ModUninitialized; // module state
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
   <a href="/dash">Dashboard</a><br>
  <form action="/get">
  <table>
    <tr><td>Acess point:</td><td> <input type="text" name="ssid" value="%ssid%"></td></tr>
    <tr><td>Pass :</td><td> <input type="password" name="pass" value="%pass%"></td></tr>
    <tr><td>GMT :</td><td> <input type="text" name="gmt" value="%gmt%"></td></tr>
    <tr><td>Daylight Savings  :</td><td> <input type="text" name="useds" value="%useds%"></td></tr>
    <tr><td>Sync Ntp:</td><td> <input type="text" name="syncntp" value="%syncntp%"></td></tr>
    <tr><td>Latitude:</td><td> <input type="text" name="lat" value="%lat%"></td></tr>
    <tr><td>Longitude:</td><td> <input type="text" name="lon" value="%lon%"></td></tr>
  </table>
    <input type="submit" value="Submit">
  </form>
  <form action="/restart">
    <input type="submit" value="Restart">
  </form>
  <form action="/recalibrate">
    <input type="submit" value="Recalibrate">
  </form>
</body></html>)rawliteral";

const char dash_page[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><body>
<a href="/">Settings</a><br>
<div>
  <table>
  <tr><td>Temperature inside: </td><td><span id="tempIns"></span></td></tr>
  <tr><td>Temperature outside: </td><td><span id="tempOut"></span></td></tr>
  <tr><td>Forecast: </td><td><span id="forecast"></span></td></tr>
  <tr><td>CO: </td><td><span id="co"></span></td></tr>
  <tr><td>Sunrise/Sunset: </td><td><span id="sun"></span></td></tr>
  <tr><td>Moonrise/Moonset: </td><td><span id="moon"></span></td></tr>
  <tr><td>Moon Angle: </td><td><span id="moon_ang"></span></td></tr>
  <table>
</div>
<script>
setInterval(function() {
  getData("tempIns");
  getData("tempOut");
  getData("forecast");
  getData("co");
  getData("sun");
  getData("moon");
  getData("moon_ang");
}, 5000); //2000mSeconds update rate

function getData(field) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById(field).innerHTML =
      this.responseText;
    }
  };
  xhttp.open("GET", field, true);
  xhttp.send();
}
</script>
</body>
</html>
)rawliteral";

AsyncWebServer server(80);

// REPLACE WITH YOUR NETWORK CREDENTIALS
// HTML web page to handle 3 input fields (input1, input2, input3)
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

String inp = "hello";
String processor(const String& var) {
    const char * ret = json_conf[var];
    return String(ret);
}

// ------------------------
// base interface 
// ------------------------

String keys[] {"ssid", "pass", "gmt", "useds", "syncntp", "log_delay", "lat", "lon"};


void send_tempIns(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    TEMP_mutex.lock();
    sprintf(buf,"Temp: %.2f C Pres: %.2f hpa Altd: %.2f m Humd: %.2f %%",
            TEMP_temp,TEMP_pres,TEMP_altd,TEMP_humd);
    TEMP_mutex.unlock();
    request->send_P(200, "text/html", buf);
    request->send_P(200, "text/html", "0");
}

void send_tempOut(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    OUTSIDE_RECIEVER_mutex.lock();
    sprintf(buf,"Temp: %.2f C Pres: %.2f hpa Humd: %.2f m UV: %.2f %%",
            data_state.temp, data_state.pres, data_state.hum, data_state.uv_idx);
    OUTSIDE_RECIEVER_mutex.unlock();
    request->send_P(200, "text/html", buf);
}


#ifdef ZAMBRETTI_MOD_H
void send_forecast(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    zambretti_mutex.lock();
    if(weather_state  & Steady)    sprintf(buf, "%s%s ",buf,"Steady");
    if(weather_state  & Falling)   sprintf(buf, "%s%s ",buf,"Falling");
    if(weather_state  & Rising)    sprintf(buf, "%s%s ",buf,"Rising");
    if(weather_state  & Worsening) sprintf(buf, "%s%s ",buf,"Worsening");
    if(weather_state  & Rainy)     sprintf(buf, "%s%s ",buf,"Rainy");
    if(weather_state  & Cloudy)    sprintf(buf, "%s%s ",buf,"Cloudy");
    if(weather_state  & Sunny)     sprintf(buf, "%s%s ",buf,"Sunny");
    zambretti_mutex.unlock();
    request->send_P(200, "text/html", buf);
}
#endif

#ifdef ZAMBRETTI_MOD_MK2_H
void send_forecast(AsyncWebServerRequest *request){
    static char buf[256];
    memset(buf, 0, 256);
    int32_t lF = 0;
    zambretti_mutex.lock();
    lF = Forecast;
    zambretti_mutex.unlock();
    sprintf(buf, "%s%s ",buf,printables[lF & 0x3].c_str());
    int sz = log2(lF);
    for (int i = 2; i <= sz; i++ ){
        if ((1 << i) & lF) {
            sprintf(buf, "%s%s ",buf,printables[i+1].c_str());
        }
    }
    request->send_P(200, "text/html", buf);
}
#endif

void send_co(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    sprintf(buf,"CO ppm:%f",GAS_ppm);
    request->send_P(200, "text/html", buf);
}

void send_sun(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    ASTRO_mutex.lock();
    sprintf(buf,"Sun Rise: %02d:%02d Sun Set: %02d:%02d",
            (SRTime.hour() + gmt + ds) % 24,SRTime.minute(),
            (SSTime.hour() + gmt + ds) % 24,SSTime.minute());
    ASTRO_mutex.unlock();
    request->send_P(200, "text/html", buf);
}

void send_moon(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    ASTRO_mutex.lock();
    sprintf(buf,"Moon Rise: %02d:%02d Moon Set: %02d:%02d",
            (MRTime.hour() + gmt + ds) % 24,MRTime.minute(),
            (MSTime.hour() + gmt + ds) % 24,MSTime.minute());
    ASTRO_mutex.unlock();
    request->send_P(200, "text/html", buf);

}

void send_moon_ang(AsyncWebServerRequest *request){
    static char buf[128];
    memset(buf, 0, 128);
    ASTRO_mutex.lock();
    sprintf(buf, "Ang: %d o Lit: %.2f ", moon_dat.angle,moon_dat.percentLit * 100  );
    ASTRO_mutex.unlock();
    request->send_P(200, "text/html", buf);
}

modState WEBSRV_init() { 
    if (WEBSRV_state) {
        server.on("/", HTTP_GET, 
                [](AsyncWebServerRequest *request){
                request->send_P(200, "text/html", index_html,processor);
                });
        server.on("/restart", HTTP_GET, 
                [](AsyncWebServerRequest *request){
                ESP.restart();
                request->send_P(200, "text/html", index_html,processor);
                });
        server.on("/recalibrate", HTTP_GET, 
                [](AsyncWebServerRequest *request){
                mq7.calibrate();
                request->send_P(200, "text/html", index_html,processor);
                });
        server.on("/dash", HTTP_GET, 
                [](AsyncWebServerRequest *request){
                request->send_P(200, "text/html", dash_page);
                });
        server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
                String inputMessage;
                String inputParam;
                for (int i =0; i < 8;i ++) {
                    if (request->hasParam(keys[i])) {
                        inputParam = keys[i];
                        inputMessage = request->getParam(keys[i])->value() ;
                        json_conf[inputParam] = inputMessage;
                    } else {
                        inputMessage = "No message sent";
                        inputParam = "none";
                    }
                }
                CONFIG_update(); // write all the values to file
                request->send_P(200, "text/html", index_html,processor);
        });
        server.on("/tempIns", HTTP_GET, send_tempIns);
        server.on("/tempOut", HTTP_GET, send_tempOut);
        server.on("/forecast", HTTP_GET, send_forecast);
        server.on("/co", HTTP_GET, send_co);
        server.on("/sun", HTTP_GET, send_sun);
        server.on("/moon", HTTP_GET, send_moon);
        server.on("/moon_ang", HTTP_GET, send_moon_ang);
        server.onNotFound(notFound);
        server.begin();
        WEBSRV_state = ModOK;
    }
    return WEBSRV_state;
}

#endif // MD_MOD_H
