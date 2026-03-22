# OBU — VANET Accident Detection System

STM32F411 Black Pill firmware for real-time vehicle accident detection and
Emergency Alert Message (EAM) transmission over LoRa 433 MHz to the RSU.

**Paired with:** RSU firmware at `github.com/vanetprojectbe/RSU`

```
Vehicle (OBU)                RSU (ESP32)              Cloud (CMS)
STM32F411          LoRa      LoRa SX1278
Sensors ──► Fusion ──────►  SIM800L / Wi-Fi ──────►  CMS API
                 433 MHz     Store & Forward
```

---

## Hardware Components

| Component      | Role                       | Interface        |
|----------------|----------------------------|------------------|
| STM32F411CEU6  | MCU (Black Pill)           | —                |
| MPU6050        | Primary accelerometer      | I2C1 PB6/PB7     |
| MPU9250        | Secondary accel + gyro     | I2C2 PB10/PB11   |
| DS18B20        | Environmental temperature  | 1-Wire PA1       |
| DS3231         | Real-time clock            | I2C1 PB6/PB7     |
| NEO-6M         | GPS location + time        | UART1 PA9/PA10   |
| MCP2515        | CAN bus (airbag + speed)   | SPI1 PB0/PB1     |
| LoRa SX1278    | 433 MHz radio TX           | SPI1 PA4/PA3/PB5 |
| SD card module | Offline log storage        | SPI1 PB12        |
| SW-420         | Vibration sensor           | GPIO PA0         |
| LM2596         | 12V → 5V buck converter    | Power            |
| AMS1117        | 5V → 3.3V LDO              | Power            |

---

## Complete Wiring

### Power chain
```
OBD2 pin 16  (+12V) ──► LM2596 IN+
OBD2 pin 4/5 (GND)  ──► LM2596 IN−  and common GND rail
LM2596 OUT+  (5V)   ──► AMS1117 IN
AMS1117 OUT  (3.3V) ──► all module VCC pins
```
Decoupling capacitors (required):
- LM2596 output  : 100µF electrolytic + 100nF ceramic in parallel
- AMS1117 output : 10µF electrolytic + 100nF ceramic in parallel
- Each module VCC: 100nF ceramic as close to pin as possible

---

### I2C1 — Wire  (PB6 = SCL,  PB7 = SDA)
4.7kΩ pull-up from PB6 → 3.3V and from PB7 → 3.3V (one pair for whole bus).

**MPU6050** (address 0x69)
```
VCC → 3.3V
GND → GND
SDA → PB7
SCL → PB6
AD0 → 3.3V    ← MUST be HIGH — sets address to 0x69
```

**DS3231 RTC** (address 0x68)
```
VCC → 3.3V
GND → GND
SDA → PB7
SCL → PB6
```

---

### I2C2 — Wire2  (PB10 = SCL,  PB11 = SDA)  — isolated bus
MPU9250 alone here to avoid 0x68 clash with DS3231 on I2C1.
4.7kΩ pull-up from PB10 → 3.3V and from PB11 → 3.3V.

**MPU9250** (address 0x68)
```
VCC  → 3.3V
GND  → GND
SDA  → PB11
SCL  → PB10
AD0  → GND    ← MUST be LOW — keeps address at 0x68
```

---

### SPI1  (PA5 = SCK,  PA6 = MISO,  PA7 = MOSI)  — shared bus

**LoRa SX1278 / Ra-02**
```
VCC  → 3.3V
GND  → GND
SCK  → PA5
MISO → PA6
MOSI → PA7
NSS  → PA4   (CS)
RST  → PA3
DIO0 → PB5
```
Never power LoRa without antenna — damages RF frontend.

**MCP2515 CAN module**
```
VCC  → 3.3V    (use 3.3V module or check onboard regulator)
GND  → GND
SCK  → PA5
MISO → PA6
MOSI → PA7
CS   → PB0
INT  → PB1
CANH → OBD2 pin 6   (CAN High)
CANL → OBD2 pin 14  (CAN Low)
```
120Ω termination resistor across CANH/CANL at MCP2515 end — required.

**SD card module**
```
VCC  → 3.3V
GND  → GND
SCK  → PA5
MISO → PA6
MOSI → PA7
CS   → PB12
```

---

### UART1 — NEO-6M GPS  (PA9 = TX,  PA10 = RX)
```
VCC → 3.3V
GND → GND
TX  → PA10   (GPS TX → MCU RX)
RX  → PA9    (GPS RX ← MCU TX)
```

---

### 1-Wire — DS18B20 Temperature  (PA1)
```
VCC → 3.3V
GND → GND
DQ  → PA1
        │
     4.7kΩ   ← mandatory pull-up
        │
      3.3V
```

---

### GPIO — SW-420 Vibration sensor  (PA0)
```
VCC → 3.3V
GND → GND
DO  → PA0
```

---

## Complete Pin Map

| Pin  | Signal              | Module                    |
|------|---------------------|---------------------------|
| PA0  | Digital input       | SW-420 vibration DO       |
| PA1  | 1-Wire DQ           | DS18B20 temperature       |
| PA3  | LoRa RST            | SX1278                    |
| PA4  | SPI CS              | LoRa SX1278               |
| PA5  | SPI SCK             | LoRa + SD + MCP2515       |
| PA6  | SPI MISO            | LoRa + SD + MCP2515       |
| PA7  | SPI MOSI            | LoRa + SD + MCP2515       |
| PA9  | UART1 TX (MCU→GPS)  | NEO-6M                    |
| PA10 | UART1 RX (GPS→MCU)  | NEO-6M                    |
| PB0  | SPI CS              | MCP2515 CAN               |
| PB1  | CAN INT             | MCP2515                   |
| PB5  | LoRa DIO0           | SX1278                    |
| PB6  | I2C1 SCL            | MPU6050 + DS3231          |
| PB7  | I2C1 SDA            | MPU6050 + DS3231          |
| PB10 | I2C2 SCL            | MPU9250 (isolated bus)    |
| PB11 | I2C2 SDA            | MPU9250 (isolated bus)    |
| PB12 | SPI CS              | SD card module            |

---

## Required Libraries

Install via Arduino IDE: Tools → Manage Libraries

| Library           | Author           | Search term       |
|-------------------|------------------|-------------------|
| LoRa              | Sandeep Mistry   | LoRa              |
| TinyGPSPlus       | Mikal Hart       | TinyGPSPlus       |
| MCP2515           | autowp           | MCP2515           |
| OneWire           | Paul Stoffregen  | OneWire           |
| DallasTemperature | Miles Burton     | DallasTemperature |

SD, Wire, SPI are built into STM32duino — no install needed.

### Board package (Arduino IDE)
1. File → Preferences → Additional boards manager URLs, add:
   `https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json`
2. Tools → Board → Boards Manager → search "STM32" → install "STM32 MCU based boards"
3. Tools → Board → STM32 MCU based boards → Generic STM32F4 series
4. Tools → Board part number → STM32F411CEUx (Black Pill)
5. Tools → Upload method → STM32CubeProgrammer (DFU)

---

## Serial Commands (115200 baud)

| Command  | Action                                     |
|----------|--------------------------------------------|
| `STATUS` | Show total / sent / unsent record counts   |
| `LIST`   | Print every stored record to serial        |
| `WIPE`   | Delete log.txt — only way to clear records |

---

## Detection Logic

1. Every 20ms: read MPU6050, MPU9250, gyro
2. Apply Kalman filter independently to each
3. Fuse: `final_acc = 0.7 × MPU6050 + 0.3 × MPU9250`
4. Compute IMU consistency score: `|MPU6050 − MPU9250|`
5. Trigger if: `fused_acc > 12 m/s²` OR `gyro > 250 °/s` OR `CAN airbag = true`
6. Track impact duration: ms that threshold stays active
7. 5-second cooldown prevents duplicate reports
8. Save to SD first (safe against power loss), then transmit over LoRa
9. Unsent SD records retried every 30 seconds
10. Records persist until operator sends `WIPE` via serial

---

## RSU Integration

The OBU transmits to the RSU (`github.com/vanetprojectbe/RSU`) using these
radio settings — both sides must be identical:

| Parameter     | Value            |
|---------------|------------------|
| Frequency     | 433 MHz          |
| Spreading factor | SF7           |
| Bandwidth     | 125 kHz          |
| Coding rate   | 4/5              |
| Sync word     | 0x34 (private)   |
| Tx power      | 17 dBm           |

The sync word `0x34` creates a private VANET channel — the RSU must have the
same sync word in its `config.h`. This prevents the RSU from picking up
unrelated public LoRa traffic.

The EAM payload is a single-line JSON string. The RSU stores received packets
in `.ndjson` format on its SD card and forwards them to the CMS API with an
`x-api-key` header. The OBU payload field names below are what the RSU parser
and CMS will receive:

| Field    | Description                           |
|----------|---------------------------------------|
| lat      | GPS latitude (6 decimal places)       |
| lon      | GPS longitude (6 decimal places)      |
| spd      | Vehicle speed km/h                    |
| acc      | Fused acceleration m/s²               |
| gyro     | Angular velocity deg/s                |
| a6050    | MPU6050 Kalman-filtered reading       |
| a9250    | MPU9250 Kalman-filtered reading       |
| cons     | IMU consistency score m/s²            |
| idur     | Impact duration ms                    |
| vib      | Vibration 0/1 (SW-420)               |
| temp     | Ambient temperature °C (DS18B20)      |
| abag     | Airbag deployed 0/1 (CAN)            |
| wdrop    | Wheel speed drop % (CAN)             |
| hh/mm/ss | Timestamp — GPS primary, RTC fallback |

---

## CAN Bus Notes

The airbag frame ID (`CAN_AIRBAG_ID`, default `0x095`) varies by manufacturer.
Common values: `0x095` (VAG/Ford), `0x0A0` (GM), `0x0B0` (Toyota).
Check your vehicle DBC file and update `CAN_AIRBAG_ID` in `config.h`.
CAN bus speed defaults to 500 kbps — adjust `CAN_SPEED` in `config.h`
to match your vehicle (common: 250 kbps or 500 kbps).

---

## Critical Rules

- All modules run at 3.3V only — never connect 5V to any pin
- MPU6050 AD0 must be HIGH (3.3V) — floating or LOW causes address clash with DS3231
- MPU9250 AD0 must be LOW (GND) — safe on isolated I2C2 bus
- DS18B20 DQ requires 4.7kΩ pull-up to 3.3V — 1-Wire will not work without it
- All module GNDs must share a single common ground tied to OBD2 pin 4/5
- LoRa antenna must be connected before powering the module
- MCP2515 requires 120Ω termination resistor across CANH/CANL
- LoRa sync word (0x34) must match RSU config.h — mismatched sync word = no communication
