# esp-homekit-HX711-occupancy-sensor
Native ESP8266 Homekit Occupancy Sensor using HX711 and load cells.

Related: Analog/ADC based sensor - https://github.com/danielhelmstedt/esp-homekit-analog-occupancy-sensor

This is an ESP8266 native Homekit firmware for reading weight from a HX711 and load cells. This firmware exposes an Occupancy sensor, as well as custom characteristics; Sensor value, Calibration factor, Threshold, and Tare. These custom characteristics are only viewable using Eve, Home+ or another third-party homekit app.

Based off the wonderful https://github.com/Mixiaoxiao/Arduino-HomeKit-ESP8266 library.
Implements WifiManager and ArduinoOTA.

# Issues
Threshold value and Calibration factor is not persistent across reboots. Threshold resets to 0 (undefined), calibration factor resets to 2000 (defined in my_accessory.c). Would appreciate help on how to implement EEPROM or SPIFFS to store values across reboots.

Homekit name is not dynamic like WifiManager AP and ArduinoOTA hostname. You will be unable to pair two devices with the same Homekit name. A workaround is just to change the name before flashing. The fix would be to implement the ESP.getChipId() function to append MAC address to name, but I have been unsuccessful in my attempts so far. Again, help welcomed.
