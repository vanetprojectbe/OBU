#include "gps.h"
void gps_init(){}
void gps_read(float *lat,float *lon,float *speed){
  *lat=19.0760; *lon=72.8777; *speed=60;
}
bool gps_get_time(GPSTime *t){
  t->hour=12; t->minute=30; t->second=45;
  return true;
}
