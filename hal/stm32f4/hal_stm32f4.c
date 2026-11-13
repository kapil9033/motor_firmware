#include "stm32f4xx_hal.h"
#include "hal_interface.h"
#include "board.h"

// Peripheral handle declarations
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim8; // For MIKROE Knob Encoder
ADC_HandleTypeDef hadc1;
DAC_HandleTypeDef hdac;
DMA_HandleTypeDef hdma_adc1;

void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_TIM1_Init(void);
void MX_TIM8_Encoder_Init(void);
void MX_ADC1_Init(void);
void MX_DAC_Init(void);

// STM32F4-specific implementation of the HAL interface
void HAL_Board_Init(void) {
    HAL_Init();
    SystemClock_Config(); // Configure for 180 MHz core clock

    MX_GPIO_Init();
    MX_TIM1_Init();         // Center-aligned PWM A/B/C on PE9, PE11, PE13 + ADC Trigger
    MX_TIM8_Encoder_Init(); // Knob Encoder on PC6, PC8
    MX_ADC1_Init();         // Phase voltages, currents, VBus, IBus, Halls via DMA
    MX_DAC_Init();          // DAC1/DAC2 on PA4, PA5 for scope telemetry
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Configure the main internal regulator output voltage
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // Initialize RCC Oscillators for 180 MHz (HSE + PLL)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 360;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    // Activate the Over-Drive mode
    HAL_PWREx_EnableOverDrive();

    // Initialize CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO Ports Clock
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOF_CLK_ENABLE();

    // 1. Bridge Enables: ENA (PD14), ENB (PD15), ENC (PF12)
    GPIO_InitStruct.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // 2. Timing LED (PB7)
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 3. Encoder Push Button (PB11) with Pull-up
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 4. Analog Pins GPIO Configuration (PA0, PA1, PA2, PA3, PC0, PC1, PC2, PC3, PF6, PF7, PF8)
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void MX_TIM1_Init(void) {
    // TIM1 Configuration for Complementary Center-Aligned PWM & ADC Trigger Output
    __HAL_RCC_TIM1_CLK_ENABLE();

    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;
    htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
    htim1.Init.Period = 4500; // ~20 kHz PWM at 180MHz timer clock
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    HAL_TIM_PWM_Init(&htim1);

    // Configure PWM Channels (PE9 -> CH1, PE11 -> CH2, PE13 -> CH3)
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3);

    // Export the timer update event for peripherals that use TIM1 TRGO.
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);
}

void MX_TIM8_Encoder_Init(void) {
    // MIKROE-3299 Quadrature Encoder Interface on PC6 (ENCA) and PC8 (ENCB)
    __HAL_RCC_TIM8_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 11;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    HAL_ADC_Init(&hadc1);

    // Configure all mapped ADC channels matching your hardware layout
    // 1. Phase Voltage A (PA0 - CH0)
    sConfig.Channel = ADC_CHANNEL_0; sConfig.Rank = 1; sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 2. Phase Voltage B (PA1 - CH1)
    sConfig.Channel = ADC_CHANNEL_1; sConfig.Rank = 2; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 3. Phase Current A (PA2 - CH2)
    sConfig.Channel = ADC_CHANNEL_2; sConfig.Rank = 3; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 4. Phase Current B (PA3 - CH3)
    sConfig.Channel = ADC_CHANNEL_3; sConfig.Rank = 4; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 5. Phase Voltage C (PF6 - CH4)
    sConfig.Channel = ADC_CHANNEL_4; sConfig.Rank = 5; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 6. Phase Current C (PF7 - CH5)
    sConfig.Channel = ADC_CHANNEL_5; sConfig.Rank = 6; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 7. Motor Hall C (PF8 - CH6)
    sConfig.Channel = ADC_CHANNEL_6; sConfig.Rank = 7; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 8. Motor Hall A (PC0 - CH10)
    sConfig.Channel = ADC_CHANNEL_10; sConfig.Rank = 8; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 9. Motor Hall B (PC1 - CH11)
    sConfig.Channel = ADC_CHANNEL_11; sConfig.Rank = 9; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 10. VBus (PC2 - CH12)
    sConfig.Channel = ADC_CHANNEL_12; sConfig.Rank = 10; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    // 11. IBus (PC3 - CH13)
    sConfig.Channel = ADC_CHANNEL_13; sConfig.Rank = 11; HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

void MX_DAC_Init(void) {
    // DAC1 (PA4) & DAC2 (PA5) for scope output
    __HAL_RCC_DAC_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    hdac.Instance = DAC;
    HAL_DAC_Init(&hdac);
}

void HAL_PWM_Start(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    // Also start TIM1 counter to begin triggering ADCs
    HAL_TIM_Base_Start(&htim1);
}

void HAL_PWM_SetDuty(uint16_t dutyA, uint16_t dutyB, uint16_t dutyC) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutyA);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, dutyB);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, dutyC);
}

void HAL_Bridge_Enable(uint8_t enable) {
    HAL_GPIO_WritePin(ENA_PORT, ENA_PIN, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ENB_PORT, ENB_PIN, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ENC_PORT, ENC_PIN, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_Toggle_TimingLED(void) {
    HAL_GPIO_TogglePin(LED_TIMING_PORT, LED_TIMING_PIN);
}
