# Esp32 weather station

## Main Part
   ### Hardware - indoor use 
   - Nodemcu Esp32s v1.1
   - Sensors:
      + Bme 280 sensor - humidity, temperature, pressure, altitude - https://www.arduino.cc/reference/en/libraries/bme280/
      + Ds3231 rtc module - https://www.arduino.cc/reference/en/libraries/ds3231/
      + MQ-7 CO sensor - https://www.arduino.cc/reference/en/libraries/sd/
      + K2162 Sd card storage module - https://www.arduino.cc/reference/en/libraries/sd/
      + 128x64 lcd display

   - Software capabilities
     + Calendar/ Clock/ World clock with other timezones? - https://www.arduino.cc/reference/en/libraries/chronos/
     + Add events to callendar + reminders
     + Moon phase - https://www.subsystems.us/uploads/9/8/9/4/98948044/moonphase.pdf
     + Calculate sunrise/sunset - https://en.wikipedia.org/wiki/Sunrise_equation
     + http server - https://lastminuteengineers.com/creating-esp32-web-server-arduino-ide/
     + Short term weather forecast - If i can't find formula might as well just
     train a model. Small NN, no middle layers should be fast to calculate on
     device anyway. Where to get data though...
     + Web interface for configuration
       + Timezone
       + Coordinates of device

   #### Web interface of device
   Web page hosted on device itself, on a http server.
   Configure device.
   Time, timezone manually ( alternatively use NTP) - https://www.arduino.cc/reference/en/libraries/ntpclient/
   + Add way to configure display layout.
     - On small lcd may be slideshow.
     - Displaying strategy must be implemented by every 'module',
       And just given a buffer abstraction when rendered.
       This will allow for slideshow/grid layout. Or both combined.
   + Actual files and paths are stored on the sd card.
   + Any verification of input is done on the client
   + Any session data stored only on client. if data is saved/flushed only then
     it should make a chnage.
   + Intermittenrt data is only in the surrrent session on the client

   ### Hardware - outdoor use 
   - Nodemcu Esp32s v1.1
   - Sensors:
   + Bme 280 sensor - humidity, temperature, pressure, altitude
   + Ds3231 rtc module (maybe) - we wont store any data on the outdoor device.
     if we get a reading it gets the current timestamp of the master device
   + UV GUVA-S12SD sensor for uv light/index
   Will report to main device to collect weather data for simple weather
   predicitons
## Optionals
### Configuration files
 - Make configuration files for the device to be able to init itself properly
   after reboot.
 - Have a base config and a optional config, if optional fails backup is used
### Ftp server to access the sd card
 - A simple ftp server to update the contents of the sd card
 - Web interface is outside firmware as to be updated separately, and no need to
   reflash device.
 - https://www.arduino.cc/reference/en/libraries/simpleftpserver/
### Integrate thunderbird format for 'event'/callendar files.
 -  Use the event format that thunderbird mail client uses for its events to
    quckly add them to our call callendar. Verification is done on client,
    send to on device server a simplified request
