# Modular Motor Control Firmware

High-performance motor-control firmware organized in layers for portability
across MCUs and inverter boards.

## Directory Structure
```text
motor_firmware/
├── include/                 # Shared board configuration
├── src/                     # Application entry point
├── hal/
│   ├── include/             # MCU-independent HAL interface
│   └── stm32f4/             # STM32F439 HAL implementation and config
├── drivers/
│   ├── drv8312/             # TI DRV8312 bridge driver
│   ├── encoder/             # Encoder driver
│   └── future_driver/       # Future driver stub
└── third_party/stm32cube/   # Bundled STM32F4 HAL and CMSIS dependencies
```

## Build

Install CMake, GNU Make, and the `arm-none-eabi-gcc` toolchain. The STM32F4 HAL
and CMSIS files needed by this project are bundled in `third_party/stm32cube`;
a separate STM32CubeF4 installation is not required.

Configure and build from the repository root:

```sh
cmake -S . -B build -DCMAKE_C_COMPILER=arm-none-eabi-gcc
cmake --build build
```

The build produces `build/libmotor_firmware.a`. If using another local
STM32CubeF4 package, pass its `Drivers` directory as `STM32CUBE_F4_ROOT` and
the directory containing `stm32f4xx_hal_conf.h` as `STM32_HAL_CONFIG_DIR`.

The archive is compile-verified but is not a flashable firmware image. Linking
a firmware ELF requires a startup file and linker script for the exact target
STM32F439 device.

The bundled STMicroelectronics HAL and CMSIS components retain their license notices
in `third_party/stm32cube/Drivers`.
