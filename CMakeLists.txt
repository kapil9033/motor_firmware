cmake_minimum_required(VERSION 3.15)

# Bare-metal toolchains cannot link CMake's hosted compiler-check executable.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(motor_firmware C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(STM32CUBE_F4_ROOT "" CACHE PATH
    "STM32CubeF4 Drivers directory containing STM32F4xx_HAL_Driver and CMSIS")
set(STM32_HAL_CONFIG_DIR "" CACHE PATH
    "Directory containing the application's stm32f4xx_hal_conf.h")

# Reuse a nearby STM32CubeF4 installation when no path is specified.
if(NOT STM32CUBE_F4_ROOT)
    set(_stm32cube_candidates
        "${CMAKE_CURRENT_SOURCE_DIR}/../stm32f446_rtos_sensor_node/Drivers"
        "${CMAKE_CURRENT_SOURCE_DIR}/../stm32_cpp20_fsm/Drivers"
    )
    foreach(_candidate IN LISTS _stm32cube_candidates)
        if(EXISTS "${_candidate}/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h")
            set(STM32CUBE_F4_ROOT "${_candidate}")
            break()
        endif()
    endforeach()
endif()

if(NOT EXISTS "${STM32CUBE_F4_ROOT}/STM32F4xx_HAL_Driver/Inc/stm32f4xx_hal.h")
    message(FATAL_ERROR
        "STM32F4 HAL not found. Set STM32CUBE_F4_ROOT to the STM32CubeF4 Drivers directory.")
endif()

if(NOT STM32_HAL_CONFIG_DIR)
    get_filename_component(_stm32cube_project_root "${STM32CUBE_F4_ROOT}" DIRECTORY)
    if(EXISTS "${_stm32cube_project_root}/Core/Inc/stm32f4xx_hal_conf.h")
        set(STM32_HAL_CONFIG_DIR "${_stm32cube_project_root}/Core/Inc")
    endif()
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
