#pragma once
struct Kalman {
  float q;
  float r;
  float x;
  float p;
  float k;
};
void kalman_init(Kalman *k, float q, float r, float initial);
float kalman_update(Kalman *k, float measurement);
