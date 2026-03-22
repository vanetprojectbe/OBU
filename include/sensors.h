#pragma once
#include <Arduino.h>

// Initialise both IMUs — call once in setup()
void imu_init();

// MPU6050 (I2C1) — primary high-frequency accelerometer
float read_acceleration();      // net m/s², gravity subtracted

// MPU9250 (I2C2) — secondary accelerometer + gyroscope
float read_acc2();              // net m/s², gravity subtracted
float read_gyro();              // magnitude deg/s

// SW-420 vibration sensor
float read_vibration();         // 0.0 = still, 1.0 = vibrating

// DS18B20 environmental temperature (1-Wire, PA1)
float read_temperature();       // °C  — returns -999.0 if sensor disconnected
