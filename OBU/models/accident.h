#pragma once
struct AccidentData {
  float acc_delta;
  float gyro_delta;
  float vibration;
  float temperature;
  float lat;
  float lon;
  float speed;
  bool airbag;
  float wheel_drop;
  int hour;
  int minute;
  int second;
};
