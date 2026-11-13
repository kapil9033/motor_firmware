#ifndef DRV8312_H
#define DRV8312_H

#include <stdint.h>

void DRV8312_Init(void);
void DRV8312_SetPowerState(uint8_t active);

#endif // DRV8312_H
