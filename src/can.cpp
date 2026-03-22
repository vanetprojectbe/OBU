#include "can.h"
#include "config.h"
#include <SPI.h>
#include <mcp2515.h>   // autowp/arduino-mcp2515

static MCP2515 _mcp(CAN_CS_PIN);

static float _baseSpeed    = 0.0f;
static float _currentSpeed = 0.0f;
static bool  _airbag       = false;

bool can_init() {
  _mcp.reset();
  if (_mcp.setBitrate(CAN_SPEED, CAN_CLOCK) != MCP2515::ERROR_OK) {
    Serial.println("[CAN] setBitrate failed — check wiring and crystal.");
    return false;
  }
  _mcp.setNormalMode();
  pinMode(CAN_INT_PIN, INPUT);
  Serial.println("[CAN] MCP2515 initialised.");
  Serial.print("[CAN] Airbag frame ID: 0x"); Serial.println(CAN_AIRBAG_ID, HEX);
  Serial.print("[CAN] Bus speed: "); Serial.println(CAN_SPEED == CAN_500KBPS ? "500 kbps" : "250 kbps");
  return true;
}

// ── OBD2 PID helpers ─────────────────────────────────────────────────────────
static void request_pid(uint8_t pid) {
  struct can_frame req;
  req.can_id  = 0x7DF;
  req.can_dlc = 8;
  req.data[0] = 0x02;
  req.data[1] = 0x01;   // Mode 01 — current data
  req.data[2] = pid;
  for (int i = 3; i < 8; i++) req.data[i] = 0xAA;
  _mcp.sendMessage(&req);
}

static bool read_response(struct can_frame &frame) {
  unsigned long t = millis();
  while (millis() - t < 5) {
    if (_mcp.readMessage(&frame) == MCP2515::ERROR_OK) return true;
  }
  return false;
}

// ── Poll — call every loop tick ───────────────────────────────────────────────
static void can_poll() {
  struct can_frame frame;

  // Drain interrupt-flagged frames (airbag broadcast, etc.)
  if (digitalRead(CAN_INT_PIN) == LOW) {
    while (_mcp.readMessage(&frame) == MCP2515::ERROR_OK) {
      if (frame.can_id == CAN_AIRBAG_ID) {
        if (frame.data[0] & 0x01) {
          _airbag = true;
          Serial.println("[CAN] Airbag deployment detected!");
        }
      }
    }
  }

  // Poll vehicle speed via OBD2 PID 0x0D
  request_pid(0x0D);
  if (read_response(frame)) {
    if (frame.can_dlc >= 4 && frame.data[1] == 0x41 && frame.data[2] == 0x0D) {
      float spd = (float)frame.data[3];  // km/h, 1 LSB = 1 km/h
      if (_baseSpeed < 1.0f) _baseSpeed = spd;
      _currentSpeed = spd;
    }
  }
}

bool can_airbag() {
  can_poll();
  return _airbag;
}

float can_speed_drop() {
  can_poll();
  if (_baseSpeed < 1.0f) return 0.0f;
  float drop = ((_baseSpeed - _currentSpeed) / _baseSpeed) * 100.0f;
  return drop < 0.0f ? 0.0f : drop;
}
