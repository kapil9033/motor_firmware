cmake_minimum_required(VERSION 3.15)

# Bare-metal toolchains cannot link CMake's hosted compiler-check executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(motor_firmware C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(STM32CUBE_F4_ROOT "" CACHE PATH
    "STM32CubeF4 Drivers directory containing STM32F4xx_HAL_Driver and CMSIS")
set(STM32_HAL_CONFIG_DIR "" CACHE PATH
    "Directory containing the application's stm32f4xx_hal_conf.h")

if(NOT STM32CUBE_F4_ROOT)
    set(STM32CUBE_F4_ROOT
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/stm32cube/Drivers")
endif()

if(NOT EXISTS "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h")
    message(FATAL_ERROR
        "STM32F4 HAL not found. Set STM32CUBE_F4_ROOT to the STM32CubeF4 Drivers directory.")
endif()

if(NOT STM32_HAL_CONFIG_DIR)
    set(STM32_HAL_CONFIG_DIR "${CMAKE_CURRENT_SOURCE_DIR}/hal/stm32f4/include")
endif()

if(NOT EXISTS "${STM32_HAL_CONFIG_DIR}/stm32f4xx_hal_conf.h")
    message(FATAL_ERROR
        "stm32f4xx_hal_conf.h not found. Set STM32_HAL_CONFIG_DIR to its directory.")
endif()

if(EXISTS "${STM32CUBE_F4_ROOT}/CMSIS/Device/ST/STM32F4xx/Include")
    set(STM32_CMSIS_DEVICE_INCLUDE
        "${STM32CUBE_F4_ROOT}/CMSIS/Device/ST/STM32F4xx/Include")
    set(STM32_CMSIS_CORE_INCLUDE "${STM32CUBE_F4_ROOT}/CMSIS/Include")
    set(STM32_CMSIS_SYSTEM_SOURCE
        "${STM32CUBE_F4_ROOT}/CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c")
elseif(EXISTS "${STM32CUBE_F4_ROOT}/CMSIS_Device/Include")
    set(STM32_CMSIS_DEVICE_INCLUDE "${STM32CUBE_F4_ROOT}/CMSIS_Device/Include")
    set(STM32_CMSIS_CORE_INCLUDE "${STM32CUBE_F4_ROOT}/CMSIS_5/CMSIS/Core/Include")
    set(STM32_CMSIS_SYSTEM_SOURCE
        "${STM32CUBE_F4_ROOT}/CMSIS_Device/Source/Templates/system_stm32f4xx.c")
else()
    message(FATAL_ERROR
        "CMSIS device headers not found under STM32CUBE_F4_ROOT.")
endif()

set(STM32_HAL_MODULES
    stm32f4xx_hal
    stm32f4xx_hal_cortex
    stm32f4xx_hal_adc
    stm32f4xx_hal_adc_ex
    stm32f4xx_hal_dac
    stm32f4xx_hal_dac_ex
    stm32f4xx_hal_dma
    stm32f4xx_hal_flash
    stm32f4xx_hal_flash_ex
    stm32f4xx_hal_gpio
    stm32f4xx_hal_pwr
    stm32f4xx_hal_pwr_ex
    stm32f4xx_hal_rcc
    stm32f4xx_hal_rcc_ex
    stm32f4xx_hal_tim
    stm32f4xx_hal_tim_ex
)

set(STM32_HAL_SOURCES)
foreach(_module IN LISTS STM32_HAL_MODULES)
    set(_source
        "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Src/${_module}.c")
    if(NOT EXISTS "${_source}")
        message(FATAL_ERROR "Required STM32 HAL source not found: ${_source}")
    endif()
    list(APPEND STM32_HAL_SOURCES "${_source}")
endforeach()

if(NOT EXISTS "${STM32_CMSIS_SYSTEM_SOURCE}")
    message(FATAL_ERROR
        "CMSIS system source not found: ${STM32_CMSIS_SYSTEM_SOURCE}")
endif()

set(SOURCES
    src/main.c
    hal/stm32f4/hal_stm32f4.c
    drivers/drv8312/drv8312.c
    "${STM32_CMSIS_SYSTEM_SOURCE}"
    ${STM32_HAL_SOURCES}
)

add_library(motor_firmware STATIC ${SOURCES})

target_include_directories(motor_firmware PRIVATE
    include
    hal/include
    drivers/drv8312
    "${STM32_HAL_CONFIG_DIR}"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/Legacy"
    "${STM32_CMSIS_DEVICE_INCLUDE}"
    "${STM32_CMSIS_CORE_INCLUDE}"
)

target_compile_definitions(motor_firmware PRIVATE
    STM32F439xx
    USE_HAL_DRIVER
)

if(CMAKE_C_COMPILER MATCHES "arm-none-eabi")
    target_compile_options(motor_firmware PRIVATE
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -Wall
        -Wextra
        -ffunction-sections
        -fdata-sections
    )
else()
    message(FATAL_ERROR
        "This firmware requires the arm-none-eabi GCC toolchain.")
endif()

set(STM32F446_SERVO_HAL_MODULES
    stm32f4xx_hal
    stm32f4xx_hal_cortex
    stm32f4xx_hal_dma
    stm32f4xx_hal_gpio
    stm32f4xx_hal_pwr
    stm32f4xx_hal_pwr_ex
    stm32f4xx_hal_rcc
    stm32f4xx_hal_rcc_ex
    stm32f4xx_hal_tim
    stm32f4xx_hal_tim_ex
)

set(STM32F446_SERVO_HAL_SOURCES)
foreach(_module IN LISTS STM32F446_SERVO_HAL_MODULES)
    set(_source
        "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Src/${_module}.c")
    if(NOT EXISTS "${_source}")
        message(FATAL_ERROR "Required STM32 HAL source not found: ${_source}")
    endif()
    list(APPEND STM32F446_SERVO_HAL_SOURCES "${_source}")
endforeach()

set(STM32F446_STARTUP_SOURCE
    "${STM32CUBE_F4_ROOT}/CMSIS/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s")
set(STM32F446_LINKER_SCRIPT
    "${CMAKE_CURRENT_SOURCE_DIR}/boards/nucleo_f446re/STM32F446RETx_FLASH.ld")

if(NOT EXISTS "${STM32F446_STARTUP_SOURCE}")
    message(FATAL_ERROR "STM32F446 startup source not found: ${STM32F446_STARTUP_SOURCE}")
endif()
if(NOT EXISTS "${STM32F446_LINKER_SCRIPT}")
    message(FATAL_ERROR "STM32F446 linker script not found: ${STM32F446_LINKER_SCRIPT}")
endif()

add_executable(sg90_nucleo_f446re.elf
    src/sg90_demo.c
    src/syscalls.c
    drivers/servo/sg90.c
    hal/nucleo_f446re/servo_pwm_stm32f446re.c
    "${STM32_CMSIS_SYSTEM_SOURCE}"
    "${STM32F446_STARTUP_SOURCE}"
    ${STM32F446_SERVO_HAL_SOURCES}
)

target_include_directories(sg90_nucleo_f446re.elf PRIVATE
    hal/include
    drivers/servo
    "${STM32_HAL_CONFIG_DIR}"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/Legacy"
    "${STM32_CMSIS_DEVICE_INCLUDE}"
    "${STM32_CMSIS_CORE_INCLUDE}"
)

target_compile_definitions(sg90_nucleo_f446re.elf PRIVATE
    STM32F446xx
    USE_HAL_DRIVER
    USER_VECT_TAB_ADDRESS
    VECT_TAB_OFFSET=0x00020200U
)

target_compile_options(sg90_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

target_link_options(sg90_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -specs=nano.specs
    -specs=nosys.specs
    "-T${STM32F446_LINKER_SCRIPT}"
    -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/sg90_nucleo_f446re.map"
)

set_property(TARGET sg90_nucleo_f446re.elf APPEND PROPERTY LINK_DEPENDS
    "${STM32F446_LINKER_SCRIPT}"
)

find_program(ARM_NONE_EABI_OBJCOPY arm-none-eabi-objcopy)
find_program(ARM_NONE_EABI_SIZE arm-none-eabi-size)
if(NOT ARM_NONE_EABI_OBJCOPY OR NOT ARM_NONE_EABI_SIZE)
    message(FATAL_ERROR "arm-none-eabi-objcopy and arm-none-eabi-size are required.")
endif()

add_custom_command(TARGET sg90_nucleo_f446re.elf POST_BUILD
    COMMAND "${ARM_NONE_EABI_SIZE}" "$<TARGET_FILE:sg90_nucleo_f446re.elf>"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex
        "$<TARGET_FILE:sg90_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/sg90_nucleo_f446re.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary
        "$<TARGET_FILE:sg90_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/sg90_nucleo_f446re.bin"
    VERBATIM
)

add_executable(d9_led_test_nucleo_f446re.elf
    src/d9_led_test.c
    src/syscalls.c
    "${STM32_CMSIS_SYSTEM_SOURCE}"
    "${STM32F446_STARTUP_SOURCE}"
    ${STM32F446_SERVO_HAL_SOURCES}
)

target_include_directories(d9_led_test_nucleo_f446re.elf PRIVATE
    "${STM32_HAL_CONFIG_DIR}"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/Legacy"
    "${STM32_CMSIS_DEVICE_INCLUDE}"
    "${STM32_CMSIS_CORE_INCLUDE}"
)

target_compile_definitions(d9_led_test_nucleo_f446re.elf PRIVATE
    STM32F446xx
    USE_HAL_DRIVER
    USER_VECT_TAB_ADDRESS
    VECT_TAB_OFFSET=0x00020200U
)

target_compile_options(d9_led_test_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

target_link_options(d9_led_test_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -specs=nano.specs
    -specs=nosys.specs
    "-T${STM32F446_LINKER_SCRIPT}"
    -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/d9_led_test_nucleo_f446re.map"
)

set_property(TARGET d9_led_test_nucleo_f446re.elf APPEND PROPERTY LINK_DEPENDS
    "${STM32F446_LINKER_SCRIPT}"
)

add_custom_command(TARGET d9_led_test_nucleo_f446re.elf POST_BUILD
    COMMAND "${ARM_NONE_EABI_SIZE}" "$<TARGET_FILE:d9_led_test_nucleo_f446re.elf>"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex
        "$<TARGET_FILE:d9_led_test_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/d9_led_test_nucleo_f446re.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary
        "$<TARGET_FILE:d9_led_test_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/d9_led_test_nucleo_f446re.bin"
    VERBATIM
)

add_executable(stepper_28byj48_nucleo_f446re.elf
    src/stepper_28byj48_demo.c
    src/syscalls.c
    "${STM32_CMSIS_SYSTEM_SOURCE}"
    "${STM32F446_STARTUP_SOURCE}"
    ${STM32F446_SERVO_HAL_SOURCES}
)

target_include_directories(stepper_28byj48_nucleo_f446re.elf PRIVATE
    "${STM32_HAL_CONFIG_DIR}"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc"
    "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/Legacy"
    "${STM32_CMSIS_DEVICE_INCLUDE}"
    "${STM32_CMSIS_CORE_INCLUDE}"
)

target_compile_definitions(stepper_28byj48_nucleo_f446re.elf PRIVATE
    STM32F446xx
    USE_HAL_DRIVER
    USER_VECT_TAB_ADDRESS
    VECT_TAB_OFFSET=0x00020200U
)

target_compile_options(stepper_28byj48_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -Wall
    -Wextra
    -ffunction-sections
    -fdata-sections
)

target_link_options(stepper_28byj48_nucleo_f446re.elf PRIVATE
    -mcpu=cortex-m4
    -mthumb
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -specs=nano.specs
    -specs=nosys.specs
    "-T${STM32F446_LINKER_SCRIPT}"
    -Wl,--gc-sections
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/stepper_28byj48_nucleo_f446re.map"
)

set_property(TARGET stepper_28byj48_nucleo_f446re.elf APPEND PROPERTY LINK_DEPENDS
    "${STM32F446_LINKER_SCRIPT}"
)

add_custom_command(TARGET stepper_28byj48_nucleo_f446re.elf POST_BUILD
    COMMAND "${ARM_NONE_EABI_SIZE}" "$<TARGET_FILE:stepper_28byj48_nucleo_f446re.elf>"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O ihex
        "$<TARGET_FILE:stepper_28byj48_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/stepper_28byj48_nucleo_f446re.hex"
    COMMAND "${ARM_NONE_EABI_OBJCOPY}" -O binary
        "$<TARGET_FILE:stepper_28byj48_nucleo_f446re.elf>"
        "${CMAKE_CURRENT_BINARY_DIR}/stepper_28byj48_nucleo_f446re.bin"
    VERBATIM
)
