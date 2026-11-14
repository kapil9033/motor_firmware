#include "stepper_interface.h"

#include "stm32f4xx_hal.h"

static GPIO_TypeDef *const input_ports[] = {
    GPIOA,
    GPIOB,
    GPIOB,
    GPIOB,
};

static const uint16_t input_pins[] = {
    GPIO_PIN_10,
    GPIO_PIN_3,
    GPIO_PIN_5,
    GPIO_PIN_4,
};

bool HAL_Stepper_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);

    gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_Stepper_SetCoils(0U);
    return true;
}

void HAL_Stepper_SetCoils(uint8_t phase_mask) {
    for (uint32_t input = 0U; input < 4U; ++input) {
        const GPIO_PinState state =
            (phase_mask & (1U << input)) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(input_ports[input], input_pins[input], state);
    }
}
