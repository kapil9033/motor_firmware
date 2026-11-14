#include "servo_pwm_interface.h"

#include "stm32f4xx_hal.h"

#define SERVO_PWM_FREQUENCY_HZ 50U
#define SERVO_PWM_TICK_FREQUENCY_HZ 1000000U
#define SERVO_PWM_PERIOD_US (1000000U / SERVO_PWM_FREQUENCY_HZ)
#define SERVO_PWM_MIN_PULSE_US 1000U
#define SERVO_PWM_MAX_PULSE_US 2000U

static TIM_HandleTypeDef servo_timer;

bool HAL_ServoPwm_Init(void) {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_7;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    gpio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOC, &gpio);

    RCC_ClkInitTypeDef clock_config = {0};
    uint32_t flash_latency = 0U;
    HAL_RCC_GetClockConfig(&clock_config, &flash_latency);

    uint32_t timer_clock_hz = HAL_RCC_GetPCLK1Freq();
    if (clock_config.APB1CLKDivider != RCC_HCLK_DIV1) {
        timer_clock_hz *= 2U;
    }

    if (timer_clock_hz < SERVO_PWM_TICK_FREQUENCY_HZ ||
        (timer_clock_hz % SERVO_PWM_TICK_FREQUENCY_HZ) != 0U) {
        return false;
    }

    servo_timer.Instance = TIM3;
    servo_timer.Init.Prescaler =
        (timer_clock_hz / SERVO_PWM_TICK_FREQUENCY_HZ) - 1U;
    servo_timer.Init.CounterMode = TIM_COUNTERMODE_UP;
    servo_timer.Init.Period = SERVO_PWM_PERIOD_US - 1U;
    servo_timer.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    servo_timer.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_PWM_Init(&servo_timer) != HAL_OK) {
        return false;
    }

    TIM_OC_InitTypeDef channel = {0};
    channel.OCMode = TIM_OCMODE_PWM1;
    channel.Pulse = 1500U;
    channel.OCPolarity = TIM_OCPOLARITY_HIGH;
    channel.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(&servo_timer, &channel, TIM_CHANNEL_2) !=
        HAL_OK) {
        return false;
    }

    return HAL_TIM_PWM_Start(&servo_timer, TIM_CHANNEL_2) == HAL_OK;
}

void HAL_ServoPwm_SetPulseUs(uint16_t pulse_us) {
    if (pulse_us < SERVO_PWM_MIN_PULSE_US) {
        pulse_us = SERVO_PWM_MIN_PULSE_US;
    } else if (pulse_us > SERVO_PWM_MAX_PULSE_US) {
        pulse_us = SERVO_PWM_MAX_PULSE_US;
    }

    __HAL_TIM_SET_COMPARE(&servo_timer, TIM_CHANNEL_2, pulse_us);
}
