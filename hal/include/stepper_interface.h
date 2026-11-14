#ifndef STEPPER_INTERFACE_H
#define STEPPER_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

bool HAL_Stepper_Init(void);
void HAL_Stepper_SetCoils(uint8_t phase_mask);

#endif
