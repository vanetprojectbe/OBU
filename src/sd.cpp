#include "sd.h"
#include "config.h"
#include "lora.h"
#include <SPI.h>
#include <SD.h>
#include <Arduino.h>

static bool   _sdReady   = false;
static String _serialBuf = "";

// ── JSON build/parse ──────────────────────────────────────────────────────────
static String build_json(const AccidentData &d, int sent) {
  String s = "{";
  s += "\"lat\":"   + String(d.lat,                  6) + ",";
  s += "\"lon\":"   + String(d.lon,                  6) + ",";
  s += "\"spd\":"   + String(d.speed,                1) + ",";
  s += "\"acc\":"   + String(d.acc_delta,            2) + ",";
  s += "\"gyro\":"  + String(d.gyro_delta,           2) + ",";
  s += "\"a6050\":" + String(d.acc_mpu6050,          2) + ",";
  s += "\"a9250\":" + String(d.acc_mpu9250,          2) + ",";
  s += "\"cons\":"  + String(d.imu_consistency_score,2) + ",";
  s += "\"idur\":"  + String(d.impact_duration_ms,   0) + ",";
  s += "\"vib\":"   + String(d.vibration,            0) + ",";
  s += "\"temp\":"  + String(d.temperature,          1) + ",";
  s += "\"abag\":"  + String(d.airbag ? 1 : 0)          + ",";
  s += "\"wdrop\":" + String(d.wheel_drop,           1) + ",";
  s += "\"hh\":"    + String(d.hour)                    + ",";
  s += "\"mm\":"    + String(d.minute)                  + ",";
  s += "\"ss\":"    + String(d.second)                  + ",";
  s += "\"sent\":"  + String(sent);
  s += "}";
  return s;
}

static float jf(const String &l, const char *k) {
  String key = String("\"") + k + "\":";
  int i = l.indexOf(key);
  if (i < 0) return 0.0f;
  return l.substring(i + key.length()).toFloat();
}
static int ji(const String &l, const char *k) { return (int)jf(l, k); }

static bool parse_line(const String &line, AccidentData &d, int &sent) {
  if (line.length() < 10) return false;
  d.lat                  = jf(line, "lat");
  d.lon                  = jf(line, "lon");
  d.speed                = jf(line, "spd");
  d.acc_delta            = jf(line, "acc");
  d.gyro_delta           = jf(line, "gyro");
  d.acc_mpu6050          = jf(line, "a6050");
  d.acc_mpu9250          = jf(line, "a9250");
  d.imu_consistency_score= jf(line, "cons");
  d.impact_duration_ms   = jf(line, "idur");
  d.vibration            = jf(line, "vib");
  d.temperature          = jf(line, "temp");
  d.airbag               = ji(line, "abag") != 0;
  d.wheel_drop           = jf(line, "wdrop");
  d.hour                 = ji(line, "hh");
  d.minute               = ji(line, "mm");
  d.second               = ji(line, "ss");
  sent                   = ji(line, "sent");
  return true;
}

// ── Public API ────────────────────────────────────────────────────────────────
bool sd_init() {
  pinMode(SD_CS_PIN, OUTPUT);
  digitalWrite(SD_CS_PIN, HIGH);
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("[SD] Init failed.");
    _sdReady = false;
    return false;
  }
  _sdReady = true;
  Serial.println("[SD] Card ready.");
  return true;
}

void sd_log_accident(const AccidentData &data) {
  if (!_sdReady) return;
  File f = SD.open(SD_LOG_FILE, FILE_WRITE);
  if (!f) { Serial.println("[SD] Cannot open log.txt."); return; }
  f.println(build_json(data, 0));
  f.close();
  Serial.println("[SD] Record saved (sent=0).");
}

void sd_retry_unsent() {
  if (!_sdReady) return;
  File f = SD.open(SD_LOG_FILE, FILE_READ);
  if (!f) return;

  String rewritten = "";
  int total = 0, retried = 0;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() < 10) continue;
    AccidentData d; int sent = 1;
    if (!parse_line(line, d, sent)) continue;
    total++;
    if (sent == 0) { send_eam(d); retried++; sent = 1; }
    rewritten += build_json(d, sent) + "\n";
  }
  f.close();

  if (retried == 0) return;
  SD.remove(SD_LOG_FILE);
  File out = SD.open(SD_LOG_FILE, FILE_WRITE);
  if (!out) { Serial.println("[SD] ERROR: rewrite failed."); return; }
  out.print(rewritten);
  out.close();
  Serial.print("[SD] Retransmitted "); Serial.print(retried);
  Serial.print("/"); Serial.println(total);
}

void sd_handle_serial() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      _serialBuf.trim(); _serialBuf.toUpperCase();
      if (_serialBuf == "WIPE") {
        if (!_sdReady) { Serial.println("[SD] No card."); }
        else if (SD.remove(SD_LOG_FILE)) Serial.println("[SD] Wiped.");
        else Serial.println("[SD] Nothing to wipe.");
      } else if (_serialBuf == "LIST") {
        if (!_sdReady) { Serial.println("[SD] No card."); }
        else {
          File f = SD.open(SD_LOG_FILE, FILE_READ);
          if (!f) { Serial.println("[SD] No records."); }
          else {
            int n = 0;
            while (f.available()) {
              String l = f.readStringUntil('\n'); l.trim();
              if (l.length() < 10) continue;
              Serial.print("["); Serial.print(++n); Serial.print("] "); Serial.println(l);
            }
            f.close();
            if (!n) Serial.println("[SD] File empty.");
          }
        }
      } else if (_serialBuf == "STATUS") {
        if (!_sdReady) { Serial.println("[SD] No card."); }
        else {
          File f = SD.open(SD_LOG_FILE, FILE_READ);
          if (!f) { Serial.println("[SD] No records yet."); }
          else {
            int total=0, sent=0, unsent=0;
            while (f.available()) {
              String l = f.readStringUntil('\n'); l.trim();
              if (l.length() < 10) continue; total++;
              if (ji(l,"sent")==1) sent++; else unsent++;
            }
            f.close();
            Serial.print("[SD] Total:"); Serial.println(total);
            Serial.print("[SD] Sent: "); Serial.println(sent);
            Serial.print("[SD] Unsent:"); Serial.println(unsent);
          }
        }
      } else if (_serialBuf.length() > 0) {
        Serial.println("Commands: WIPE | LIST | STATUS");
      }
      _serialBuf = "";
    } else { _serialBuf += c; }
  }
}
