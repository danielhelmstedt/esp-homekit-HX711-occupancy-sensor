#include <ArduinoOTA.h>
#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <HX711.h>

HX711 scale;
#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

const int LOADCELL_DOUT_PIN = D4; // Remember these are ESP GPIO pins, they are not the physical pins on the board.
const int LOADCELL_SCK_PIN = D3;

void setup() {
  Serial.begin(115200);
<<<<<<< HEAD
  Serial.println( );

  Serial.println("HX711 Homekit Sensor. DOUT_PIN = D4, SCK_PIN = D3");

  //Create dynamic hostname
=======
  
>>>>>>> parent of f36b7c7... Implemented EEPROM
  char out[20];
  sprintf(out, "PressureSensor-%X",ESP.getChipId());
  const char * serial_str = out;
  Serial.println(serial_str);

  Serial.println("HX711 Homekit Sensor. DOUT_PIN = D4, SCK_PIN = D3");
  
  pinMode(LED_BUILTIN, OUTPUT); 
  digitalWrite(LED_BUILTIN, HIGH); // turn the LED off.
  
  WiFiManager wifiManager;
  wifiManager.autoConnect(serial_str);

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

// access your homekit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_occupancy;
extern "C" homekit_characteristic_t cha_threshold;
extern "C" homekit_characteristic_t cha_calibration;
extern "C" homekit_characteristic_t cha_sensorValue;
extern "C" homekit_characteristic_t cha_tare;

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_setup() {
  cha_tare.setter = tare_callback;
    
  arduino_homekit_setup(&config);
}

void my_homekit_loop() {
  arduino_homekit_loop();
  const uint32_t t = millis();
  if (t > next_report_millis) {
    // report sensor values every 5 seconds
    next_report_millis = t + 1 * 1000;
    homekit_report();
  }
  if (t > next_heap_millis) {
    // show heap info every 5 seconds
    next_heap_millis = t + 5 * 1000;
    LOG_D("Free heap: %d, HomeKit clients: %d",
          ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

  }
}

void homekit_report() {
<<<<<<< HEAD
  //Read EEPROM
  if(cha_threshold.value.int_value == 0 && data.eepromThreshold != 0) { //If homekit threshold is 0 and EEPROM has data saved
    cha_threshold.value.int_value = data.eepromThreshold;
    homekit_characteristic_notify(&cha_threshold, cha_threshold.value);
    Serial.println("Updated Homekit threshold with saved value");
  }
  // if(cha_calibration.value.int_value == 0 && data.eepromCalibration != 0) { //If homekit threshold is 0 and EEPROM has data saved
  //   cha_calibration.value.int_value = data.eepromCalibration;
  //   homekit_characteristic_notify(&cha_calibration, cha_calibration.value);
  //   Serial.println("Updated Homekit threshold with saved value");
  // }
=======
  int calibration_factor = cha_calibration.value.int_value;
>>>>>>> parent of f36b7c7... Implemented EEPROM
  
  scale.set_scale(cha_calibration.value.int_value); //Adjust to this calibration factor

  Serial.print("Reading: ");
  int reading = (scale.get_units(3));
  if (reading < 0) { reading = reading * -1;}
  Serial.print(reading);
  Serial.print(" kgs"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  Serial.print(" Calibration factor: ");
  Serial.print(cha_calibration.value.int_value);
  Serial.println();

  uint8_t occupancy = reading >= cha_threshold.value.int_value ? 1 : 0; // Logic - Implements threshold
  
  cha_occupancy.value.uint8_value = occupancy;
  cha_sensorValue.value.int_value = reading;
  homekit_characteristic_notify(&cha_occupancy, cha_occupancy.value);
  homekit_characteristic_notify(&cha_sensorValue, cha_sensorValue.value);
  
  LOG_D("occupancy %u", occupancy);
}

void tare_callback(const homekit_value_t v) {
    Serial.println("Tare Scale");
    scale.tare();
    delay(50);
    cha_tare.value.bool_value = 0; //turn the switch off again
    homekit_characteristic_notify(&cha_tare, cha_tare.value);
}
