#include "hal_interface.h"
#include "drv8312.h"
#include "board.h"

int main(void) {
    // 1. Initialize Hardware Abstraction & Board Peripherals
    HAL_Board_Init();

    // 2. Initialize Power Stage Driver
    DRV8312_Init();
    DRV8312_SetPowerState(1); // Enable bridge

    // 3. Start PWM generation
    HAL_PWM_Start();

    while (1) {
        // Main control loop / background tasks
        HAL_Toggle_TimingLED();
    }
}
