#include "sensors.h"
#include "config.h"
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ── DS18B20 1-Wire temperature sensor ────────────────────────────────────────
static OneWire           _ow(DS18B20_PIN);
static DallasTemperature _ds(&_ow);

// ── Registers ─────────────────────────────────────────────────────────────────
#define ACCEL_XOUT_H   0x3B
#define GYRO_XOUT_H    0x43
#define PWR_MGMT_1     0x6B
#define ACCEL_CONFIG   0x1C   // ±8g range → 0x10
#define GYRO_CONFIG    0x1B   // ±2000 dps → 0x18

// I2C2 instance for MPU9250 (isolated bus — avoids 0x68 clash with DS3231 RTC)
TwoWire Wire2(IMU2_SDA_PIN, IMU2_SCL_PIN);

static const float ONE_G = 9.80665f;

// ── Helper ────────────────────────────────────────────────────────────────────
static void read_xyz(TwoWire &bus, uint8_t addr, uint8_t reg,
                     int16_t &x, int16_t &y, int16_t &z) {
  bus.beginTransmission(addr);
  bus.write(reg);
  bus.endTransmission(false);
  bus.requestFrom(addr, (uint8_t)6);
  x = (int16_t)((bus.read() << 8) | bus.read());
  y = (int16_t)((bus.read() << 8) | bus.read());
  z = (int16_t)((bus.read() << 8) | bus.read());
}

static void write_reg(TwoWire &bus, uint8_t addr, uint8_t reg, uint8_t val) {
  bus.beginTransmission(addr);
  bus.write(reg);
  bus.write(val);
  bus.endTransmission();
}

// ── Init ──────────────────────────────────────────────────────────────────────
void imu_init() {
  Wire.begin();
  Wire2.begin();

  // MPU6050 — wake, set ±8g accel range for impact detection
  write_reg(Wire,  MPU6050_ADDR, PWR_MGMT_1,   0x00);
  write_reg(Wire,  MPU6050_ADDR, ACCEL_CONFIG,  0x10); // ±8g → 4096 LSB/g
  delay(10);

  // MPU9250 — wake, set ±8g accel, ±2000dps gyro
  write_reg(Wire2, MPU9250_ADDR, PWR_MGMT_1,   0x00);
  write_reg(Wire2, MPU9250_ADDR, ACCEL_CONFIG,  0x10); // ±8g
  write_reg(Wire2, MPU9250_ADDR, GYRO_CONFIG,   0x18); // ±2000dps → 16.4 LSB/dps
  delay(10);

  pinMode(VIBRATION_PIN, INPUT);

  // DS18B20 — start 1-Wire bus, trigger first async conversion
  _ds.begin();
  _ds.setResolution(12);          // 12-bit = 0.0625°C resolution (750ms conversion)
  _ds.setWaitForConversion(false);// non-blocking — we read on next loop tick
  _ds.requestTemperatures();

  Serial.println("[IMU] MPU6050 + MPU9250 initialised.");
  Serial.print("[DS18B20] Devices found: ");
  Serial.println(_ds.getDeviceCount());
}

// ── MPU6050 Accelerometer ─────────────────────────────────────────────────────
float read_acceleration() {
  int16_t ax, ay, az;
  read_xyz(Wire, MPU6050_ADDR, ACCEL_XOUT_H, ax, ay, az);
  // ±8g range → 4096 LSB/g
  float mag = sqrtf((float)(ax*ax + ay*ay + az*az)) / 4096.0f;
  return fabsf((mag * ONE_G) - ONE_G);
}

// ── MPU9250 Accelerometer ─────────────────────────────────────────────────────
float read_acc2() {
  int16_t ax, ay, az;
  read_xyz(Wire2, MPU9250_ADDR, ACCEL_XOUT_H, ax, ay, az);
  float mag = sqrtf((float)(ax*ax + ay*ay + az*az)) / 4096.0f;
  return fabsf((mag * ONE_G) - ONE_G);
}

// ── MPU9250 Gyroscope ─────────────────────────────────────────────────────────
float read_gyro() {
  int16_t gx, gy, gz;
  read_xyz(Wire2, MPU9250_ADDR, GYRO_XOUT_H, gx, gy, gz);
  // ±2000dps → 16.4 LSB/dps
  return sqrtf((float)(gx*gx + gy*gy + gz*gz)) / 16.4f;
}

// ── SW-420 Vibration ──────────────────────────────────────────────────────────
float read_vibration() {
  return (float)digitalRead(VIBRATION_PIN);
}

// ── DS18B20 Environmental Temperature ────────────────────────────────────────
// Non-blocking pattern: request conversion, read on the next call.
// At 12-bit resolution conversion takes ~750ms — much longer than our 20ms
// loop, so we always read the previous result and immediately request the next.
float read_temperature() {
  float tempC = _ds.getTempCByIndex(0);

  // Request next conversion immediately (non-blocking)
  _ds.requestTemperatures();

  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("[DS18B20] Sensor disconnected or not found.");
    return -999.0f;   // sentinel — receiver should treat this as invalid
  }
  return tempC;
}
