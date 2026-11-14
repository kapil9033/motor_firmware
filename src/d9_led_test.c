#include "stm32f4xx_hal.h"

#define D9_GPIO_PORT GPIOC
#define D9_GPIO_PIN GPIO_PIN_7
#define LED_INTERVAL_MS 500U

void SysTick_Handler(void) {
    HAL_IncTick();
}

int main(void) {
    HAL_Init();

    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = D9_GPIO_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(D9_GPIO_PORT, &gpio);

    for (;;) {
        HAL_GPIO_TogglePin(D9_GPIO_PORT, D9_GPIO_PIN);
        HAL_Delay(LED_INTERVAL_MS);
    }
}
