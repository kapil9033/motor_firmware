#include "stm32f4xx_hal.h"

#define STEPS_PER_OUTPUT_REVOLUTION 4096U
#define STEP_INTERVAL_MS 4U
#define DIRECTION_PAUSE_MS 1000U

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

static const uint8_t half_step_sequence[] = {
    0x1U,
    0x3U,
    0x2U,
    0x6U,
    0x4U,
    0xCU,
    0x8U,
    0x9U,
};

void SysTick_Handler(void) {
    HAL_IncTick();
}

static void set_coils(uint8_t pattern) {
    for (uint32_t input = 0U; input < 4U; ++input) {
        const GPIO_PinState state =
            (pattern & (1U << input)) != 0U ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(input_ports[input], input_pins[input], state);
    }
}

static void rotate_steps(uint8_t reverse) {
    for (uint32_t step = 0U; step < STEPS_PER_OUTPUT_REVOLUTION; ++step) {
        const uint32_t sequence_index =
            reverse != 0U
                ? (sizeof(half_step_sequence) - (step % sizeof(half_step_sequence))) %
                      sizeof(half_step_sequence)
                : step % sizeof(half_step_sequence);
        set_coils(half_step_sequence[sequence_index]);
        HAL_Delay(STEP_INTERVAL_MS);
    }

    set_coils(0U);
}

int main(void) {
    HAL_Init();

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
    set_coils(0U);

    for (;;) {
        rotate_steps(0U);
        HAL_Delay(DIRECTION_PAUSE_MS);
        rotate_steps(1U);
        HAL_Delay(DIRECTION_PAUSE_MS);
    }
}
