#include "encoder.h"
#include "stm32f4xx_hal.h"

extern TIM_HandleTypeDef htim8; // Defined in HAL layer for TIM8 encoder

static volatile uint8_t button_pressed_flag = 0;

void Encoder_Init(void) {
    // TIM8 Encoder mode initialization is handled in hal_stm32f4.c
    // Here we start the encoder interface
    HAL_TIM_Encoder_Start(&htim8, TIM_CHANNEL_ALL);
}

int32_t Encoder_GetCount(void) {
    // Read current quadrature counter value
    return (int32_t)__HAL_TIM_GET_COUNTER(&htim8);
}

void Encoder_ResetCount(void) {
    __HAL_TIM_SET_COUNTER(&htim8, 0);
}

uint8_t Encoder_GetButtonState(void) {
    uint8_t state = button_pressed_flag;
    button_pressed_flag = 0; // Clear flag on read
    return state;
}

// GPIO Interrupt callback for the encoder push button (PB11)
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == GPIO_PIN_11) {
        button_pressed_flag = 1;
    }
}
