#pragma once
#include "accident.h"

bool sd_init();
void sd_log_accident(const AccidentData &data);
void sd_retry_unsent();
void sd_handle_serial();
