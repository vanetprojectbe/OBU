#pragma once
#include <Arduino.h>

// MPU6050
float read_acceleration();

// MPU9250
float read_acc2();
float read_gyro();

// Other sensors
float read_vibration();
float read_temperature();
