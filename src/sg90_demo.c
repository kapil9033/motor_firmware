#include "sg90.h"

#include "stm32f4xx_hal.h"

#define SWEEP_MIN_ANGLE_DEGREES 10
#define SWEEP_MAX_ANGLE_DEGREES 170
#define SWEEP_STEP_INTERVAL_MS 20U

static void Error_Handler(void) {
    __disable_irq();
    for (;;) {
    }
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

int main(void) {
    HAL_Init();

    if (!SG90_Init()) {
        Error_Handler();
    }

    int16_t angle_degrees = SWEEP_MIN_ANGLE_DEGREES;
    int16_t direction = 1;
    uint32_t last_step_tick = HAL_GetTick();

    if (!SG90_SetAngle((uint16_t)angle_degrees)) {
        Error_Handler();
    }

    for (;;) {
        const uint32_t current_tick = HAL_GetTick();
        if ((uint32_t)(current_tick - last_step_tick) <
            SWEEP_STEP_INTERVAL_MS) {
            continue;
        }

        last_step_tick = current_tick;
        angle_degrees += direction;

        if (angle_degrees >= SWEEP_MAX_ANGLE_DEGREES) {
            angle_degrees = SWEEP_MAX_ANGLE_DEGREES;
            direction = -1;
        } else if (angle_degrees <= SWEEP_MIN_ANGLE_DEGREES) {
            angle_degrees = SWEEP_MIN_ANGLE_DEGREES;
            direction = 1;
        }

        if (!SG90_SetAngle((uint16_t)angle_degrees)) {
            Error_Handler();
        }
    }
}
