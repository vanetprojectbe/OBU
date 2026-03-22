#pragma once
#include "accident.h"

void lora_init();
void send_eam(const AccidentData &data);
