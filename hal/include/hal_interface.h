#ifndef HAL_INTERFACE_H
#define HAL_INTERFACE_H

#include <stdint.h>

// Abstract HAL interface.
// If you change MCUs, implement these exact functions for the new MCU.
void HAL_Board_Init(void);
void HAL_PWM_Start(void);
void HAL_PWM_SetDuty(uint16_t dutyA, uint16_t dutyB, uint16_t dutyC);
void HAL_Bridge_Enable(uint8_t enable);
void HAL_Toggle_TimingLED(void);

#endif // HAL_INTERFACE_H
