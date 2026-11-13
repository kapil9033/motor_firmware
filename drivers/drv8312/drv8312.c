#include "drv8312.h"
#include "hal_interface.h"

// TI DRV8312EVM Specific Control & Fault handling
void DRV8312_Init(void) {
    // Configure fault inputs, reset lines specific to DRV8312
    HAL_Bridge_Enable(0); // Start disabled for safety
}

void DRV8312_SetPowerState(uint8_t active) {
    HAL_Bridge_Enable(active);
}
