/*
 * Future Motor Driver / Inverter Implementation Stub
 * Swap this in when changing the power electronics board.
 */
#include "future_driver.h"
#include "hal_interface.h"

void FutureDriver_Init(void) {
    // TODO: Initialize communication (SPI/UART) or fault lines for new driver
    HAL_Bridge_Enable(0);
}

void FutureDriver_SetPowerState(uint8_t active) {
    HAL_Bridge_Enable(active);
}
