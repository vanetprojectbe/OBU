#include "lora.h"
#include "config.h"
#include <SPI.h>
#include <LoRa.h>

void lora_init() {
  LoRa.setPins(LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN);
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("[LoRa] Init failed — check wiring and antenna!");
    return;
  }

  // ── Radio settings — MUST match RSU exactly ───────────────────────────────
  // RSU uses Sandeep Mistry LoRa library defaults:
  //   SF7, BW 125 kHz, CR 4/5, sync word 0x12
  // Any mismatch = OBU and RSU cannot communicate.
  LoRa.setSpreadingFactor(7);       // SF7 — matches RSU default
  LoRa.setSignalBandwidth(125E3);   // 125 kHz — matches RSU default
  LoRa.setCodingRate4(5);           // 4/5 — matches RSU default
  LoRa.setSyncWord(LORA_SYNC_WORD); // 0x34 — private VANET network
  LoRa.setTxPower(17);              // 17 dBm

  Serial.println("[LoRa] SX1278 ready.");
  Serial.print("  Freq   : "); Serial.print(LORA_FREQ / 1E6, 0); Serial.println(" MHz");
  Serial.print("  SF     : "); Serial.println(7);
  Serial.print("  BW     : 125 kHz");  Serial.println();
  Serial.print("  SyncW  : 0x");       Serial.println(LORA_SYNC_WORD, HEX);
}

void send_eam(const AccidentData &d) {
  // Build JSON payload — field names agreed with RSU lora_vanet.cpp parser
  String p = "{";
  p += "\"lat\":"   + String(d.lat,                  6) + ",";
  p += "\"lon\":"   + String(d.lon,                  6) + ",";
  p += "\"spd\":"   + String(d.speed,                1) + ",";
  p += "\"acc\":"   + String(d.acc_delta,            2) + ",";
  p += "\"gyro\":"  + String(d.gyro_delta,           2) + ",";
  p += "\"a6050\":" + String(d.acc_mpu6050,          2) + ",";
  p += "\"a9250\":" + String(d.acc_mpu9250,          2) + ",";
  p += "\"cons\":"  + String(d.imu_consistency_score,2) + ",";
  p += "\"idur\":"  + String(d.impact_duration_ms,   0) + ",";
  p += "\"vib\":"   + String(d.vibration,            0) + ",";
  p += "\"temp\":"  + String(d.temperature,          1) + ",";
  p += "\"abag\":"  + String(d.airbag ? 1 : 0)          + ",";
  p += "\"wdrop\":" + String(d.wheel_drop,           1) + ",";
  p += "\"hh\":"    + String(d.hour)                    + ",";
  p += "\"mm\":"    + String(d.minute)                  + ",";
  p += "\"ss\":"    + String(d.second);
  p += "}";

  LoRa.beginPacket();
  LoRa.print(p);
  LoRa.endPacket();

  Serial.print("[LoRa] EAM sent (");
  Serial.print(p.length());
  Serial.println(" bytes).");
  Serial.println(p);   // echo payload to serial for debugging
}
