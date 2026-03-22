#pragma once

// ════════════════════════════════════════════════════════════════════════════
//  OBU — Master Configuration
//  Board  : STM32F411CEU6 (Black Pill)
//  Partner: RSU — ESP32 + LoRa SX1278 (github.com/vanetprojectbe/RSU)
// ════════════════════════════════════════════════════════════════════════════

// ── Accident Detection Thresholds ────────────────────────────────────────────
#define ACC_THRESHOLD          12.0f   // m/s² net (gravity subtracted)
#define GYRO_THRESHOLD        250.0f   // deg/s
#define IMU_CONSISTENCY_MAX    5.0f    // m/s² — max acceptable diff between IMUs

// ── Sensor Fusion Weights ─────────────────────────────────────────────────────
#define FUSION_W_MPU6050       0.7f    // MPU6050 weight (fast impact)
#define FUSION_W_MPU9250       0.3f    // MPU9250 weight (rotation context)

// ── LoRa (SX1278) ─────────────────────────────────────────────────────────────
// All radio settings MUST match RSU (github.com/vanetprojectbe/RSU) exactly.
// RSU uses Sandeep Mistry LoRa library — defaults are SF7, BW 125kHz, CR 4/5.
// Sync word 0x34 = private VANET network (prevents cross-talk with public LoRa).
#define LORA_FREQ              433E6
#define LORA_SYNC_WORD         0x34   // private VANET — must match RSU config.h
#define LORA_CS_PIN            PA4
#define LORA_RST_PIN           PA3
#define LORA_DIO0_PIN          PB5

// ── SPI1 shared bus (LoRa + SD + MCP2515) ────────────────────────────────────
//  PA5 = SCK, PA6 = MISO, PA7 = MOSI

// ── SD Card ───────────────────────────────────────────────────────────────────
#define SD_CS_PIN              PB12
#define SD_LOG_FILE            "/log.txt"
#define LORA_RETRY_INTERVAL_MS 30000UL

// ── MCP2515 CAN ───────────────────────────────────────────────────────────────
#define CAN_CS_PIN             PB0
#define CAN_INT_PIN            PB1
#define CAN_SPEED              CAN_500KBPS   // adjust to match vehicle (250/500 kbps)
#define CAN_CLOCK              MCP_8MHZ      // MCP2515 crystal — 8 MHz standard

// Airbag CAN frame ID — varies by manufacturer.
// Common: 0x095 (VAG/Ford), 0x0A0 (GM), 0x0B0 (Toyota).
// Check vehicle DBC file and update this value.
#define CAN_AIRBAG_ID          0x095

// ── GPS (NEO-6M) — UART1 ──────────────────────────────────────────────────────
//  PA9  = TX (MCU → GPS)
//  PA10 = RX (GPS → MCU)
#define GPS_BAUD               9600

// ── I2C Address Map ───────────────────────────────────────────────────────────
//
//  I2C1 (Wire)   PB6=SCL  PB7=SDA
//    MPU6050  0x69  AD0 → 3.3V (HIGH)
//    DS3231   0x68  fixed
//
//  I2C2 (Wire2)  PB10=SCL  PB11=SDA  ← isolated, avoids 0x68 clash with RTC
//    MPU9250  0x68  AD0 → GND (LOW)

#define MPU6050_ADDR           0x69
#define MPU9250_ADDR           0x68
#define RTC_ADDR               0x68

#define IMU2_SCL_PIN           PB10
#define IMU2_SDA_PIN           PB11

// ── GPIO ──────────────────────────────────────────────────────────────────────
#define VIBRATION_PIN          PA0    // SW-420 DO pin
#define DS18B20_PIN            PA1    // DS18B20 DQ (4.7kΩ pull-up to 3.3V required)

// ── Timing ────────────────────────────────────────────────────────────────────
#define LOOP_DELAY_MS          20
#define ACCIDENT_COOLDOWN_MS   5000UL
#define IMPACT_DURATION_MAX_MS 2000UL
