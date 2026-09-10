# SoC-дефиниция: STM32F103x6 (семейство STM32F1, Cortex-M3)
#
# Единственный источник фактов об этом SoC (R3). Каталог добавляется в
# ABL_SOC_INDEX (cmake/helpers/soc.cmake) по значению mcu.part.

set(ABL_SOC_FAMILY     f1)
set(ABL_SOC_DEVICE     STM32F103x6)      # device define для vendor-HAL
set(ABL_SOC_HAL_PREFIX stm32f1xx)        # префикс файлов HAL SDK
set(ABL_SOC_CORE       cortex-m3)
set(ABL_SOC_FPU        "")               # у F1 нет FPU
set(ABL_SOC_FLOAT_ABI  soft)

# Файлы bring-up — относительно этого каталога
set(ABL_SOC_STARTUP startup_stm32f103x6.s)
set(ABL_SOC_LINKER  STM32F103X6_FLASH.ld)
set(ABL_SOC_SYSTEM  system_stm32f1xx.c)
