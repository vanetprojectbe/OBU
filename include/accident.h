#pragma once

struct AccidentData {
  // IMU
  float acc_delta;
  float gyro_delta;
  float acc_mpu6050;
  float acc_mpu9250;
  float imu_consistency_score;
  float impact_duration_ms;
  // Sensors
  float vibration;
  float temperature;
  // GPS
  float lat;
  float lon;
  float speed;
  // CAN
  bool  airbag;
  float wheel_drop;
  // Time
  int   hour;
  int   minute;
  int   second;
};
