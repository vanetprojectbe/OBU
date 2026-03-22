#pragma once

struct Kalman { float q, r, x, p, k; };

void  kalman_init(Kalman *k, float q, float r, float initial);
float kalman_update(Kalman *k, float measurement);
