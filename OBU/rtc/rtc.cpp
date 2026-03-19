#include <Wire.h>
#define RTC_ADDR 0x68
void rtc_init(){ Wire.begin(); }
void rtc_set_time(uint8_t h,uint8_t m,uint8_t s){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0); Wire.write(s); Wire.write(m); Wire.write(h);
  Wire.endTransmission();
}
void rtc_get_time(uint8_t *h,uint8_t *m,uint8_t *s){
  Wire.beginTransmission(RTC_ADDR);
  Wire.write(0); Wire.endTransmission();
  Wire.requestFrom(RTC_ADDR,3);
  *s=Wire.read(); *m=Wire.read(); *h=Wire.read();
}
