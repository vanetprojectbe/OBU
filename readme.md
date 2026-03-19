Here’s your complete, production-grade README.md for the OBU 🚀
You can directly paste this into your repo.

⸻

:::writing{variant=“standard” id=“84217”}

🚗 VANET On-Board Unit (OBU)

📌 Overview

The On-Board Unit (OBU) is the core detection system of the VANET-based accident response platform. It is installed inside a vehicle and is responsible for:
	•	Detecting accidents in real-time
	•	Performing sensor fusion (dual IMU + CAN + environmental data)
	•	Generating a structured Emergency Alert Message (EAM)
	•	Broadcasting alerts to nearby Roadside Units (RSUs) via LoRa

This system is designed for low latency (<3 seconds) and high reliability, ensuring rapid emergency response.

⸻

🧠 System Architecture

Sensors → STM32 MCU → Sensor Fusion → Feature Extraction → LoRa → RSU → CMS → Emergency Services


⸻

🔩 Hardware Components

🧠 Microcontroller
	•	STM32F411CEU6 (Black Pill)

⸻

📡 Communication
	•	LoRa SX1278 (433 MHz)

⸻

🧭 Sensors

Sensor	Purpose
MPU6050	High-frequency impact detection
MPU9250	Orientation & angular velocity
SW420	Vibration detection
MLX90614	Temperature (fire detection)
NEO-6M	GPS (location + time)


⸻

🚗 Vehicle Interface
	•	MCP2515 CAN Module

⸻

⏱️ Timing
	•	DS3231 RTC Module

⸻

⚡ Power
	•	LM2596 Buck Converter (12V → 5V)
	•	AMS1117 (5V → 3.3V)

⸻

🔌 Wiring

🟢 I2C Bus

Device	Address	Notes
MPU9250	0x68	Default
MPU6050	0x69	AD0 → 3.3V
MLX90614	0x5A	Fixed
DS3231	0x68	RTC

PB6 → SCL  
PB7 → SDA  


⸻

🔵 SPI Bus

PA5 → SCK  
PA6 → MISO  
PA7 → MOSI  

LoRa SX1278

CS → PA4  
RST → PA3  
DIO0 → PA2  

MCP2515 CAN

CS → PB0  
INT → PB1  


⸻

🟡 UART (GPS)

TX → PA2  
RX → PA3  


⸻

🔴 Vibration Sensor

OUT → PB10  


⸻

⚡ Power

12V → LM2596 → 5V → AMS1117 → 3.3V


⸻

⚙️ Software Architecture

OBU/
├── main.cpp
├── config.h
├── imu/
├── kalman/
├── gps/
├── rtc/
├── can/
├── lora/
├── sensors/


⸻

🧠 Sensor Fusion Strategy

Dual IMU Design

Sensor	Role
MPU6050	Fast impact trigger
MPU9250	Rotation & rollover detection


⸻

Fusion Logic
	1.	Kalman Filtering
	2.	Consistency Check
	3.	Weighted Fusion

final_acc = 0.7 * MPU6050 + 0.3 * MPU9250


⸻

Trigger Conditions

IF:
  acceleration > threshold
  OR gyro > threshold
  OR airbag deployed

THEN:
  Accident detected


⸻

📊 Feature Extraction

The OBU generates the following features:

[
  acc_delta,
  gyro_delta,
  vibration_intensity,
  impact_duration,
  airbag_deployed,
  wheel_speed_drop_pct,
  thermal_c,
  latitude,
  longitude,
  initial_speed,
  imu_consistency_score
]


⸻

📡 Emergency Alert Message (EAM)

Example:

{
  "lat": 19.0760,
  "lon": 72.8777,
  "acc": 18.5,
  "gyro": 320.1,
  "temp": 45.2,
  "airbag": true
}


⸻

⏱️ Time Synchronization

Priority:
	1.	GPS time (primary)
	2.	RTC DS3231 (fallback)

⸻

🚀 How It Works
	1.	Sensors continuously stream data
	2.	MPU6050 detects high G-force
	3.	System performs synchronous read
	4.	Kalman filter + fusion applied
	5.	Features extracted
	6.	EAM generated
	7.	LoRa transmits to RSU

⸻

📡 VANET Communication
	•	Protocol: LoRa (SX1278)
	•	Frequency: 433 MHz
	•	Range: Up to 10 km (line-of-sight)

⸻

🔒 Reliability Features
	•	Dual IMU redundancy
	•	CAN-based validation
	•	GPS + RTC time sync
	•	Noise filtering (Kalman)
	•	Multi-trigger logic

⸻

⚠️ Important Notes
	•	STM32 operates at 3.3V ONLY
	•	Always connect common ground
	•	Never power LoRa without antenna
	•	Ensure CAN bus termination (120Ω)

⸻

🧪 Testing

Basic Test
	•	Shake device → detect vibration
	•	Simulate impact → trigger LoRa

Advanced Test
	•	Simulate CAN signals
	•	Validate GPS lock
	•	Check RTC fallback

⸻

🔮 Future Improvements
	•	Extended Kalman Filter (EKF)
	•	Edge ML inference
	•	Vehicle-to-vehicle (V2V) communication
	•	OTA firmware updates

⸻

🏁 Conclusion

This OBU provides:
	•	⚡ Real-time accident detection
	•	🧠 Intelligent sensor fusion
	•	📡 Reliable VANET communication
	•	🚑 Immediate emergency response capability

⸻

👨‍💻 Author

Final Year Project – VANET Accident Detection System
:::

⸻

If you want next level, I can also generate:
	•	📦 Full GitHub repo (OBU + RSU + CMS integrated)
	•	📄 IEEE paper version of this README
	•	🔧 PCB + enclosure design
	•	🧪 Testing + validation plan (for viva/demo)

Just say 👍