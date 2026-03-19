#include <SPI.h>
#include <LoRa.h>
#include "../config.h"
#include "lora.h"
void lora_init(){
  LoRa.setPins(4,3,2);
  LoRa.begin(LORA_FREQ);
}
void send_eam(AccidentData d){
  String payload="{";
  payload+="\"lat\":"+String(d.lat)+",";
  payload+="\"lon\":"+String(d.lon)+",";
  payload+="\"acc\":"+String(d.acc_delta)+",";
  payload+="\"gyro\":"+String(d.gyro_delta)+",";
  payload+="\"temp\":"+String(d.temperature);
  payload+="}";
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();
}
