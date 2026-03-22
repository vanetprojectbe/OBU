#pragma once

struct GPSTime { int hour; int minute; int second; };

void  gps_init();
void  gps_read(float *lat, float *lon, float *speed);
bool  gps_get_time(GPSTime *t);
bool  gps_has_fix();
