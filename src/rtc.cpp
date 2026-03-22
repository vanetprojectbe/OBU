#include "rtc.h"
#include "config.h"
#include <Wire.h>

// DS3231 BCD helpers
static uint8_t bcd2dec(uint8_t b) { return (b >> 4) * 10 + (b & 0x0F); }
static uint8_t dec2bcd(uint8_t d) { return ((d / 10) << 4) | (d % 10); }

void rtc_init() {
  // Wire already started by imu_init(); safe to call begin() again
  Wire.begin();
  // Disable DS3231 oscillator stop flag / enable oscillator
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x0E);   // control register
  Wire.write(0x00);   // RS2=0, RS1=0, INTCN=0, A2IE=0, A1IE=0 — normal run
  Wire.endTransmission();
  Serial.println("[RTC] DS3231 initialised.");
}

void rtc_set_time(uint8_t h, uint8_t m, uint8_t s) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  Wire.write(dec2bcd(s));
  Wire.write(dec2bcd(m));
  Wire.write(dec2bcd(h));
  Wire.endTransmission();
}

void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s) {
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(RTC_ADDR, (uint8_t)3);
  *s = bcd2dec(Wire.read() & 0x7F);  // mask out oscillator-stop bit
  *m = bcd2dec(Wire.read());
  *h = bcd2dec(Wire.read() & 0x3F);  // mask out 12/24h bit
}
