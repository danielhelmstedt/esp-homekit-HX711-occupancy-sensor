#include <ArduinoOTA.h>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <HX711.h>
#include <ESP_EEPROM.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

HX711 scale;
const int LOADCELL_DOUT_PIN = D4;
const int LOADCELL_SCK_PIN = D3;

// access homekit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_occupancy;
extern "C" homekit_characteristic_t cha_threshold;
extern "C" homekit_characteristic_t cha_calibration;
extern "C" homekit_characteristic_t cha_sensorValue;
extern "C" homekit_characteristic_t cha_tare;

//Data to save to EEPROM
struct { 
  int eepromThreshold;
  int eepromCalibration;
} data;
uint addr = 0;

void setup() {
  Serial.begin(115200);
  Serial.println( );

  Serial.println("HX711 Homekit Sensor. DOUT_PIN = D4, SCK_PIN = D3");

  //Create dynamic hostname
  char out[20];
  sprintf(out, "HX711Sensor-%X",ESP.getChipId());
  const char * serial_str = out;
  Serial.println(serial_str);
  
  //Read EEPROM
  EEPROM.begin(512);  //Initialize EEPROM
  EEPROM.get(addr, data);
  Serial.print("Reading saved EEPROM Threshold - ");
  Serial.println(data.eepromThreshold);
  Serial.print("Reading saved EEPROM Calibration Factor - ");
  Serial.println(data.eepromCalibration);

  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED off.

  WiFiManager wifiManager;
  wifiManager.autoConnect(serial_str);
  WiFi.hostname(serial_str);

  ArduinoOTA.setHostname(serial_str);
  ArduinoOTA.setPassword("12041997");
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);   // Start scale on specified pins
  scale.wait_ready();                                 //Ensure scale is ready, this is a blocking function
  scale.set_scale();
  Serial.println("Scale Set");
  scale.wait_ready();
  scale.tare();                                       // Tare scale on startup
  scale.wait_ready();
  Serial.println("Scale Zeroed");

  my_homekit_setup();
}

void loop() {
  ArduinoOTA.handle();
  my_homekit_loop();
  delay(10);
}

//==============================
// Homekit setup and loop
//==============================

void my_homekit_setup() {
  cha_tare.setter = tare_callback;
  arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_report_millis) {
    // report sensor values every 1 seconds
    next_report_millis = t + 1 * 1000;
    homekit_report();
  }
  if (t > next_heap_millis) {
    // show heap info every 15 seconds
    next_heap_millis = t + 15 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

  }
}

/////////////////////////////////////////////////////////////////////
////////Function for reading sensor and reporting to HomeKit.////////
/////////////////////////////////////////////////////////////////////

void homekit_report() { 

  //Initialize HomeKit Calibration value from EEPROM on boot
  if(cha_calibration.value.int_value == 200 && data.eepromCalibration != 0) { 
    cha_calibration.value.int_value = data.eepromCalibration;
    homekit_characteristic_notify(&cha_calibration, cha_calibration.value);
    Serial.println("Updated HomeKit with saved Calibration value");
  }
  //Initialize HomeKit Threshold value from EEPROM on boot
  if(cha_threshold.value.int_value == 0 && data.eepromThreshold != 0) { 
    cha_threshold.value.int_value = data.eepromThreshold;
    homekit_characteristic_notify(&cha_threshold, cha_threshold.value);
    Serial.println("Updated HomeKit with saved Threshold value");
  }
  
  //Initialize scale
  scale.set_scale(cha_calibration.value.int_value) ;

  //Read the HX711 load cells
  int reading;
  reading = (scale.get_units(3));     //Read from the sensor
  if (reading < 0) {reading = reading * -1;} //force min
  if (reading > 200) {reading = 200;} //force max

  //Update HomeKit
  cha_sensorValue.value.int_value = reading;
  cha_occupancy.value.bool_value = reading <= cha_threshold.value.int_value ? 0 : 1; //Determine occupancy status
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);
  homekit_characteristic_notify(&cha_sensorValue, cha_sensorValue.value);
  Serial.print("Calibration factor: ");
  Serial.println(cha_calibration.value.int_value);
  Serial.print("threshold = ");
  Serial.println(cha_threshold.value.int_value);
  Serial.print("sensor reading = ");
  Serial.println(reading);
  Serial.print("occupancy = ");
  Serial.println(cha_occupancy.value.bool_value); 

  //Update EEPROM
  if(cha_calibration.value.int_value != data.eepromCalibration){
    data.eepromCalibration = cha_calibration.value.int_value; //sync values 
    EEPROM.put(addr, data.eepromCalibration);  //prepare changes, if any
    EEPROM.commit();                           //Perform write to flash
    Serial.println("Got Calibration factor change - updated EEPROM");
  }
  if(cha_threshold.value.int_value != data.eepromThreshold){
    data.eepromThreshold = cha_threshold.value.int_value; //sync values
    EEPROM.put(addr, data);  //prepare changes, if any
    EEPROM.commit();         //Perform write to flash
    Serial.println("Got Threshold change - updating EEPROM");
  }
}

void tare_callback(const homekit_value_t v) {
    Serial.println("Tare Scale");
    scale.tare();
    cha_tare.value.bool_value = 0; //turn the switch off again
    homekit_characteristic_notify(&cha_tare, cha_tare.value);
}
