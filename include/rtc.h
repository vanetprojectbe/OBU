#pragma once
#include <Arduino.h>

void rtc_init();
void rtc_set_time(uint8_t h, uint8_t m, uint8_t s);
void rtc_get_time(uint8_t *h, uint8_t *m, uint8_t *s);
