#include <Arduino.h>
#include "config.h"
#include "models/accident.h"
#include "sensors/sensors.h"
#include "gps/gps.h"
#include "rtc/rtc.h"
#include "kalman/kalman.h"
#include "lora/lora.h"
#include "can/can.h"

Kalman accFilter;
Kalman gyroFilter;

void setup(){
  Serial.begin(115200);
  rtc_init();
  gps_init();
  lora_init();
  kalman_init(&accFilter,0.01,0.1,0);
  kalman_init(&gyroFilter,0.01,0.1,0);
}

void loop(){
  float rawAcc=read_acceleration();
  float rawGyro=read_gyro();

  float acc=kalman_update(&accFilter,rawAcc);
  float gyro=kalman_update(&gyroFilter,rawGyro);

  if(acc>ACC_THRESHOLD){
    AccidentData data;
    data.acc_delta=acc;
    data.gyro_delta=gyro;
    data.vibration=read_vibration();
    data.temperature=read_temperature();

    gps_read(&data.lat,&data.lon,&data.speed);
    data.airbag=can_airbag();
    data.wheel_drop=can_speed_drop();

    GPSTime t;
    if(gps_get_time(&t)){
      rtc_set_time(t.hour,t.minute,t.second);
    }

    rtc_get_time((uint8_t*)&data.hour,(uint8_t*)&data.minute,(uint8_t*)&data.second);

    send_eam(data);
    Serial.println("Accident detected and sent");
  }
  delay(20);
}
