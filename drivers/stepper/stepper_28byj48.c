#include "stepper_28byj48.h"

#include "stepper_interface.h"

#define HALF_STEP_COUNT 8U

static const unsigned char half_step_sequence[HALF_STEP_COUNT] = {
    0x1U,
    0x3U,
    0x2U,
    0x6U,
    0x4U,
    0xCU,
    0x8U,
    0x9U,
};

static unsigned char current_phase;

bool Stepper28BYJ48_Init(void) {
    current_phase = 0U;
    if (!HAL_Stepper_Init()) {
        return false;
    }

    HAL_Stepper_SetCoils(0U);
    return true;
}

void Stepper28BYJ48_Step(bool clockwise) {
    if (clockwise) {
        current_phase = (unsigned char)((current_phase + 1U) % HALF_STEP_COUNT);
    } else {
        current_phase =
            (unsigned char)((current_phase + HALF_STEP_COUNT - 1U) %
                            HALF_STEP_COUNT);
    }

    HAL_Stepper_SetCoils(half_step_sequence[current_phase]);
}

void Stepper28BYJ48_Release(void) {
    HAL_Stepper_SetCoils(0U);
}
