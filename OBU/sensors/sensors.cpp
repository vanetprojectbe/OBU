#include "sensors.h"
#include <Wire.h>

// I2C addresses
#define MPU6050_ADDR 0x69   // AD0 → HIGH
#define MPU9250_ADDR 0x68

// Registers
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H  0x43
#define PWR_MGMT_1   0x6B

// ---------------- INIT ----------------
void imu_init() {
  Wire.begin();

  // Wake MPU6050
  Wire.beginTransmission(MPU6050_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();

  // Wake MPU9250
  Wire.beginTransmission(MPU9250_ADDR);
  Wire.write(PWR_MGMT_1);
  Wire.write(0);
  Wire.endTransmission();
}

// ---------------- READ RAW ----------------
int16_t read16(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);

  Wire.requestFrom(addr, (uint8_t)2);
  return (Wire.read() << 8) | Wire.read();
}

// ---------------- MPU6050 ACC ----------------
float read_acceleration() {
  int16_t ax = read16(MPU6050_ADDR, ACCEL_XOUT_H);
  int16_t ay = read16(MPU6050_ADDR, ACCEL_XOUT_H + 2);
  int16_t az = read16(MPU6050_ADDR, ACCEL_XOUT_H + 4);

  float acc = sqrt(ax*ax + ay*ay + az*az) / 16384.0; // g-force
  return acc * 9.81; // convert to m/s²
}

// ---------------- MPU9250 ACC ----------------
float read_acc2() {
  int16_t ax = read16(MPU9250_ADDR, ACCEL_XOUT_H);
  int16_t ay = read16(MPU9250_ADDR, ACCEL_XOUT_H + 2);
  int16_t az = read16(MPU9250_ADDR, ACCEL_XOUT_H + 4);

  float acc = sqrt(ax*ax + ay*ay + az*az) / 16384.0;
  return acc * 9.81;
}

// ---------------- MPU9250 GYRO ----------------
float read_gyro() {
  int16_t gx = read16(MPU9250_ADDR, GYRO_XOUT_H);
  int16_t gy = read16(MPU9250_ADDR, GYRO_XOUT_H + 2);
  int16_t gz = read16(MPU9250_ADDR, GYRO_XOUT_H + 4);

  float gyro = sqrt(gx*gx + gy*gy + gz*gz) / 131.0; // deg/s
  return gyro;
}

// ---------------- VIBRATION ----------------
float read_vibration() {
  return digitalRead(10);
}

// ---------------- TEMPERATURE ----------------
float read_temperature() {
  return 36.5; // placeholder (replace with MLX90614 later)
}