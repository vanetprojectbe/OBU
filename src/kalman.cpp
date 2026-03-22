#include "kalman.h"

void kalman_init(Kalman *k, float q, float r, float initial) {
  k->q = q; k->r = r; k->x = initial; k->p = 1.0f;
}

float kalman_update(Kalman *k, float measurement) {
  k->p += k->q;
  k->k  = k->p / (k->p + k->r);
  k->x += k->k * (measurement - k->x);
  k->p *= (1.0f - k->k);
  return k->x;
}
