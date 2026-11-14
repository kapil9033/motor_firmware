#ifndef SERVO_PWM_INTERFACE_H
#define SERVO_PWM_INTERFACE_H

#include <stdbool.h>
#include <stdint.h>

bool HAL_ServoPwm_Init(void);
void HAL_ServoPwm_SetPulseUs(uint16_t pulse_us);

#endif
