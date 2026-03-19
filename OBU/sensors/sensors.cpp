#include "sensors.h"
float read_acceleration(){ return analogRead(A0)*0.1; }
float read_gyro(){ return analogRead(A1)*0.1; }
float read_vibration(){ return digitalRead(2); }
float read_temperature(){ return 36.5; }
