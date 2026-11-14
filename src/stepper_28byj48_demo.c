#include "stm32f4xx_hal.h"
#include "stepper_28byj48.h"

#define STEPS_PER_OUTPUT_REVOLUTION 4096U
#define STEP_INTERVAL_MS 4U
#define DIRECTION_PAUSE_MS 1000U

void SysTick_Handler(void) {
    HAL_IncTick();
}

static void rotate_steps(bool clockwise) {
    for (uint32_t step = 0U; step < STEPS_PER_OUTPUT_REVOLUTION; ++step) {
        Stepper28BYJ48_Step(clockwise);
        HAL_Delay(STEP_INTERVAL_MS);
    }

    Stepper28BYJ48_Release();
}

int main(void) {
    HAL_Init();

    if (!Stepper28BYJ48_Init()) {
        __disable_irq();
        for (;;) {
        }
    }

    for (;;) {
        rotate_steps(true);
        HAL_Delay(DIRECTION_PAUSE_MS);
        rotate_steps(false);
        HAL_Delay(DIRECTION_PAUSE_MS);
    }
}
