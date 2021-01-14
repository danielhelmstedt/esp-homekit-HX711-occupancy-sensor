/*
   my_accessory.c
   Define the accessory in C language using the Macro in characteristics.h

  Based on the work by Mixiaoxiao (Wang Bin)
*/

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <Arduino.h>

void my_accessory_identify(homekit_value_t _value) {
  printf("accessory identify\n");
  for (int i = 0; i <= 5; i++) { //start at 0, run loop and add 1, repeat until 5
    digitalWrite(LED_BUILTIN, LOW);// turn the LED on.(Note that LOW = LED on; this is because it is active low on the ESP8266.
    delay(100);            // wait for 0.1 second.
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off.
    delay(100); // wait for 0.1 second.
  }
}

// format: uint8; 0 ”Occupancy is not detected”, 1 ”Occupancy is detected”
homekit_characteristic_t cha_occupancy = HOMEKIT_CHARACTERISTIC_(OCCUPANCY_DETECTED, 0);

//Sensor Threshold. format: int32; 1 to 1024
homekit_characteristic_t cha_threshold = HOMEKIT_CHARACTERISTIC_(CUSTOM,
    .description = "Threshold",
    .type = "151B3360-16F8-4C57-8CBA-F68B526C527A",
    .format = homekit_format_int,
    .permissions = homekit_permissions_paired_read
                   | homekit_permissions_paired_write
                   | homekit_permissions_notify,
    .min_value = (float[]) {0},
    .max_value = (float[]) {200},
    .min_step =  (float[]) {1},
);

//Sensor Value. format: int32; 1 to 1024
homekit_characteristic_t cha_sensorValue = HOMEKIT_CHARACTERISTIC_(CUSTOM,
    .description = "Sensor",
    .type = "ECC64460-AEA2-409B-BF2A-32270BB11954",
    .format = homekit_format_int,
    .permissions = homekit_permissions_paired_read
                 | homekit_permissions_notify,
    .min_value = (float[]) {0},
    .max_value = (float[]) {200},
    .min_step =  (float[]) {1},
);

//Tare switch. format: bool
homekit_characteristic_t cha_tare = HOMEKIT_CHARACTERISTIC_(CUSTOM,
         .type = "7CE55747-8174-4953-9B93-E21B4FC333CF",
         .description = "Tare",
         .format = homekit_format_bool,
         .permissions = homekit_permissions_paired_read
                      | homekit_permissions_paired_write
                      | homekit_permissions_notify,
        .value = HOMEKIT_BOOL_(false), 
);

//Calibration Factor. format: int16; 1 to 1024
homekit_characteristic_t cha_calibration = HOMEKIT_CHARACTERISTIC_(CUSTOM,
    .description = "Calibration Factor",
    .type = "755E8E97-2433-41DD-A9B9-CDC71272D2F4",
    .format = homekit_format_int,
    .permissions = homekit_permissions_paired_read
                 | homekit_permissions_paired_write
                 | homekit_permissions_notify,
    .min_value = (float[]) {0},
    .max_value = (float[]) {5000},
    .min_step =  (float[]) {10},
);

homekit_accessory_t *accessories[] = {
  HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_sensor, .services=(homekit_service_t*[]) {
      HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
      HOMEKIT_CHARACTERISTIC(NAME, "HX711 Occupancy Sensor"),
      HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Dan Helmstedt"),
      HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "00001"),
      HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266 HX711"),
      HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
      HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
      NULL
    }),
      HOMEKIT_SERVICE(OCCUPANCY_SENSOR, .primary=true, .characteristics=(homekit_characteristic_t*[]){
      &cha_occupancy,
      &cha_sensorValue,
      &cha_threshold,
      &cha_calibration,
      &cha_tare,
      NULL
    }),
    NULL
  }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "120-41-997"
};
