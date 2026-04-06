# cmake/toolchains/stm32-gcc.cmake
# Тулчейн для STM32 с использованием GCC

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ─── Toolchain resolution (priority order) ───────────────────────────────────
# 1. ABL_TOOLCHAIN_PATH env var
# 2. ~/.local/share/abl-mcu-toolchains/arm-none-eabi/current
# 3. System PATH (default)

if(DEFINED ENV{ABL_TOOLCHAIN_PATH})
    set(TOOLCHAIN_PREFIX "$ENV{ABL_TOOLCHAIN_PATH}/bin")
elseif(EXISTS "$ENV{HOME}/.local/share/abl-mcu-toolchains/arm-none-eabi/current/bin/arm-none-eabi-gcc")
    set(TOOLCHAIN_PREFIX "$ENV{HOME}/.local/share/abl-mcu-toolchains/arm-none-eabi/current/bin")
else()
    set(TOOLCHAIN_PREFIX "")
endif()

if(TOOLCHAIN_PREFIX)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}/arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}/arm-none-eabi-gcc)
    set(CMAKE_OBJCOPY ${TOOLCHAIN_PREFIX}/arm-none-eabi-objcopy)
    set(CMAKE_SIZE ${TOOLCHAIN_PREFIX}/arm-none-eabi-size)
else()
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
    set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
    set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
    set(CMAKE_SIZE arm-none-eabi-size)
endif()

# Флаги компиляции для STM32
set(STACK_SIZE "unlimited")

# Если не определен процессор, используем значение по умолчанию
if(NOT DEFINED STM32_MCU)
    set(STM32_MCU "cortex-m4")
endif()

# Если не определена FPU, используем значение по умолчанию
if(NOT DEFINED STM32_FPU)
    set(STM32_FPU "fpv4-sp-d16")
endif()

# Если не определен тип float ABI, используем значение по умолчанию
if(NOT DEFINED STM32_FLOAT_ABI)
    set(STM32_FLOAT_ABI "hard")
endif()

set(COMMON_FLAGS "-mcpu=${STM32_MCU} -mthumb -mfpu=${STM32_FPU} -mfloat-abi=${STM32_FLOAT_ABI} -ffunction-sections -fdata-sections -fno-common -fmessage-length=0")

set(CMAKE_C_FLAGS_INIT "${COMMON_FLAGS} -std=gnu11")
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=gnu++17 -fno-rtti -fno-exceptions -fno-use-cxa-atexit")
set(CMAKE_ASM_FLAGS_INIT "${COMMON_FLAGS} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,-gc-sections,--print-memory-usage -T ${CMAKE_BINARY_DIR}/generated/linker_script.ld -Wl,-Map=${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.map")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)