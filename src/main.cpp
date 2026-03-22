#include <Arduino.h>
#include "config.h"
#include "accident.h"
#include "sensors.h"
#include "gps.h"
#include "rtc.h"
#include "kalman.h"
#include "lora.h"
#include "can.h"
#include "sd.h"

// ── Kalman filters ────────────────────────────────────────────────────────────
static Kalman kf_acc6050;   // MPU6050 acceleration
static Kalman kf_acc9250;   // MPU9250 acceleration
static Kalman kf_gyro;      // MPU9250 gyroscope

// ── State ─────────────────────────────────────────────────────────────────────
static unsigned long lastAccidentMs  = 0;
static unsigned long lastRetryMs     = 0;
static unsigned long impactStartMs   = 0;
static bool          inImpact        = false;

// ── Sensor fusion ─────────────────────────────────────────────────────────────
// Weighted blend of both filtered IMU readings.
// Returns fused acceleration and populates individual + consistency fields.
static float fuse_imu(float a6050, float a9250,
                      float &out6050, float &out9250,
                      float &consistency) {
  out6050      = a6050;
  out9250      = a9250;
  consistency  = fabsf(a6050 - a9250);
  if (consistency > IMU_CONSISTENCY_MAX) {
    // Large disagreement — one IMU may be faulty or mounted differently.
    // Log the score but still use the weighted fusion; the receiver can
    // decide to trust/distrust based on this value.
    Serial.print("[FUSION] IMU inconsistency: ");
    Serial.println(consistency, 2);
  }
  return FUSION_W_MPU6050 * a6050 + FUSION_W_MPU9250 * a9250;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("================================================");
  Serial.println("  OBU — VANET Accident Detection System");
  Serial.println("  STM32F411 Black Pill");
  Serial.println("================================================");
  Serial.println("Commands: WIPE | LIST | STATUS");

  imu_init();
  rtc_init();
  gps_init();

  if (!can_init()) {
    Serial.println("[WARN] CAN unavailable — airbag/speed data disabled.");
  }

  lora_init();

  if (!sd_init()) {
    Serial.println("[WARN] SD unavailable — offline logging disabled.");
  }

  // Kalman: q=process noise, r=measurement noise, initial=0
  kalman_init(&kf_acc6050, 0.01f, 0.1f, 0.0f);
  kalman_init(&kf_acc9250, 0.01f, 0.1f, 0.0f);
  kalman_init(&kf_gyro,    0.01f, 0.1f, 0.0f);

  Serial.println("[OBU] Ready — monitoring for accidents.");
  Serial.println("================================================");
}

// ── Loop ──────────────────────────────────────────────────────────────────────
void loop() {
  // ── Serial command handler ──────────────────────────────────────────────────
  sd_handle_serial();

  // ── Read + filter all sensors ───────────────────────────────────────────────
  float raw6050 = read_acceleration();
  float raw9250 = read_acc2();
  float rawGyro = read_gyro();

  float f6050 = kalman_update(&kf_acc6050, raw6050);
  float f9250 = kalman_update(&kf_acc9250, raw9250);
  float fGyro = kalman_update(&kf_gyro,    rawGyro);

  float imu6050_out, imu9250_out, consistency;
  float fusedAcc = fuse_imu(f6050, f9250, imu6050_out, imu9250_out, consistency);

  bool airbag     = can_airbag();
  bool triggerAcc = fusedAcc > ACC_THRESHOLD;
  bool triggerGyr = fGyro    > GYRO_THRESHOLD;
  bool triggered  = triggerAcc || triggerGyr || airbag;

  // ── Impact duration tracking ─────────────────────────────────────────────────
  unsigned long now = millis();
  if (triggered && !inImpact) {
    inImpact      = true;
    impactStartMs = now;
  } else if (!triggered && inImpact) {
    inImpact = false;
  }

  // ── Accident detection + cooldown ───────────────────────────────────────────
  bool cooldownOver = (now - lastAccidentMs) >= ACCIDENT_COOLDOWN_MS;

  if (triggered && cooldownOver) {
    lastAccidentMs = now;

    float impactDuration = inImpact
      ? (float)(now - impactStartMs)
      : (float)min((unsigned long)IMPACT_DURATION_MAX_MS, now - impactStartMs);

    // Build the full EAM data record
    AccidentData data;
    data.acc_delta            = fusedAcc;
    data.gyro_delta           = fGyro;
    data.acc_mpu6050          = imu6050_out;
    data.acc_mpu9250          = imu9250_out;
    data.imu_consistency_score= consistency;
    data.impact_duration_ms   = impactDuration;
    data.vibration            = read_vibration();
    data.temperature          = read_temperature();
    data.airbag               = airbag;
    data.wheel_drop           = can_speed_drop();

    gps_read(&data.lat, &data.lon, &data.speed);

    // Time sync: GPS first, RTC fallback
    GPSTime gt;
    if (gps_get_time(&gt)) {
      rtc_set_time((uint8_t)gt.hour, (uint8_t)gt.minute, (uint8_t)gt.second);
      data.hour   = gt.hour;
      data.minute = gt.minute;
      data.second = gt.second;
    } else {
      uint8_t h, m, s;
      rtc_get_time(&h, &m, &s);
      data.hour = h; data.minute = m; data.second = s;
      Serial.println("[OBU] GPS time unavailable — using RTC fallback.");
    }

    // Log trigger reason
    Serial.print("[OBU] ACCIDENT DETECTED — triggers:");
    if (triggerAcc) Serial.print(" ACC");
    if (triggerGyr) Serial.print(" GYRO");
    if (airbag)     Serial.print(" AIRBAG");
    Serial.println();
    Serial.print("       fused_acc="); Serial.print(fusedAcc, 2);
    Serial.print(" gyro=");            Serial.print(fGyro, 1);
    Serial.print(" consistency=");     Serial.print(consistency, 2);
    Serial.print(" duration=");        Serial.print(impactDuration, 0);
    Serial.println("ms");

    // 1 — Persist to SD (survives LoRa failure / power loss)
    sd_log_accident(data);

    // 2 — Transmit immediately over LoRa
    send_eam(data);
  }

  // ── Periodic LoRa retry of unsent SD records (every 30 s) ──────────────────
  if ((now - lastRetryMs) >= LORA_RETRY_INTERVAL_MS) {
    lastRetryMs = now;
    sd_retry_unsent();
  }

  delay(LOOP_DELAY_MS);
}
