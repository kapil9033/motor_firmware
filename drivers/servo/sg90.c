#include "sg90.h"

#include "servo_pwm_interface.h"

#define SG90_MAX_ANGLE_DEGREES 180U
#define SG90_MIN_PULSE_US 1000U
#define SG90_MAX_PULSE_US 2000U
#define SG90_CENTER_ANGLE_DEGREES 90U

bool SG90_Init(void) {
    if (!HAL_ServoPwm_Init()) {
        return false;
    }

    return SG90_SetAngle(SG90_CENTER_ANGLE_DEGREES);
}

bool SG90_SetAngle(uint16_t angle_degrees) {
    if (angle_degrees > SG90_MAX_ANGLE_DEGREES) {
        return false;
    }

    const uint32_t pulse_range_us = SG90_MAX_PULSE_US - SG90_MIN_PULSE_US;
    const uint32_t pulse_us =
        SG90_MIN_PULSE_US +
        ((uint32_t)angle_degrees * pulse_range_us) / SG90_MAX_ANGLE_DEGREES;

    HAL_ServoPwm_SetPulseUs((uint16_t)pulse_us);
    return true;
}
