# Modular Motor Control Firmware

High-performance motor control firmware structured with a strict Layered Architecture for scalability across different MCUs and inverter boards.

## Directory Structure
```text
motor_firmware/
├── include/           # Central configuration (board.h)
├── src/               # Application logic and main entry point
├── hal/               # Hardware Abstraction Layer
│   ├── include/       # Abstract HAL interface definitions
│   ├── stm32f4/       # Current MCU implementation (STM32F439)
│   └── stm32g4/       # Future MCU stub (STM32G4)
├── drivers/           # Power electronics and bridge drivers
│   ├── drv8312/       # Current driver (TI DRV8312EVM)
│   └── future_driver/ # Future driver stub
└── README.md~
```

## Build

The CMake build compiles the firmware and its STM32F4 HAL dependencies into
`libmotor_firmware.a`. It uses a nearby STM32CubeF4 installation when found;
otherwise, set `STM32CUBE_F4_ROOT` to its `Drivers` directory and
`STM32_HAL_CONFIG_DIR` to the directory containing `stm32f4xx_hal_conf.h`.
Configure with the `arm-none-eabi-gcc` toolchain.

The archive is compile-verified but is not a flashable firmware image. A
device-specific startup file and linker script are needed to link an ELF for
the exact STM32F439 part.
