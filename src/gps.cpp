#include "gps.h"
#include "config.h"
#include <Arduino.h>
#include <TinyGPS++.h>

// NEO-6M is on UART1: PA9=TX(MCU→GPS), PA10=RX(GPS→MCU)
// STM32duino exposes UART1 as Serial1
static TinyGPSPlus _gps;

void gps_init() {
  Serial1.begin(GPS_BAUD);
  Serial.println("[GPS] NEO-6M initialised on UART1 (PA9/PA10).");
}

// Feed all available NMEA bytes into TinyGPS++ parser
static void _feed() {
  unsigned long start = millis();
  while (Serial1.available() && (millis() - start) < 200) {
    _gps.encode(Serial1.read());
  }
}

bool gps_has_fix() {
  _feed();
  return _gps.location.isValid() && _gps.location.age() < 2000;
}

void gps_read(float *lat, float *lon, float *speed) {
  _feed();
  if (_gps.location.isValid()) {
    *lat   = (float)_gps.location.lat();
    *lon   = (float)_gps.location.lng();
    *speed = _gps.speed.isValid() ? (float)_gps.speed.kmph() : 0.0f;
  } else {
    Serial.println("[GPS] No fix — coordinates unavailable.");
    *lat = *lon = *speed = 0.0f;
  }
}

bool gps_get_time(GPSTime *t) {
  _feed();
  if (_gps.time.isValid() && _gps.time.age() < 2000) {
    t->hour   = _gps.time.hour();
    t->minute = _gps.time.minute();
    t->second = _gps.time.second();
    return true;
  }
  return false;
}
