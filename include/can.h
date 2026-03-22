#pragma once
#include <Arduino.h>

// Initialise MCP2515 CAN controller.
// Returns true on success.
bool can_init();

// Returns true if the airbag deployment flag is set on the CAN bus.
bool can_airbag();

// Returns wheel speed drop as a percentage (0–100).
// Derived from comparing pre-event speed to current speed via CAN PID 0x0D.
float can_speed_drop();
