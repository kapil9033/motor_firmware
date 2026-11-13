#ifndef BOARD_H
#define BOARD_H

// --- PIN DEFINITIONS & HARDWARE MAPPING ---
// Centralized here so changing pins or boards doesn't break application logic.

// PWM Outputs (TIM1)
#define PWM_A_PIN       GPIO_PIN_9
#define PWM_A_PORT      GPIOE
#define PWM_B_PIN       GPIO_PIN_11
#define PWM_B_PORT      GPIOE
#define PWM_C_PIN       GPIO_PIN_13
#define PWM_C_PORT      GPIOE

// Bridge Enables (DRV8312)
#define ENA_PIN         GPIO_PIN_14
#define ENA_PORT        GPIOD
#define ENB_PIN         GPIO_PIN_15
#define ENB_PORT        GPIOD
#define ENC_PIN         GPIO_PIN_12
#define ENC_PORT        GPIOF

// Timing LED
#define LED_TIMING_PIN  GPIO_PIN_7
#define LED_TIMING_PORT GPIOB

#endif // BOARD_H
