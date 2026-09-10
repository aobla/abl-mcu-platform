# SoC-дефиниция: STM32H743xx (семейство STM32H7, Cortex-M7)
#
# Единственный источник фактов об этом SoC (R3). Каталог добавляется в
# ABL_SOC_INDEX (cmake/helpers/soc.cmake) по значению mcu.part.

set(ABL_SOC_FAMILY     h7)
set(ABL_SOC_DEVICE     STM32H743xx)      # device define для vendor-HAL
set(ABL_SOC_HAL_PREFIX stm32h7xx)        # префикс файлов HAL SDK
set(ABL_SOC_CORE       cortex-m7)
set(ABL_SOC_FPU        fpv5-sp-d16)
set(ABL_SOC_FLOAT_ABI  hard)

# Файлы bring-up — относительно этого каталога
set(ABL_SOC_STARTUP startup_stm32h743xx.s)
set(ABL_SOC_LINKER  STM32H743XI_FLASH.ld)
set(ABL_SOC_SYSTEM  system_stm32h7xx.c)
