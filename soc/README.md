# soc

Подъём платформы (bring-up) по SoC (D1) + **единственный источник фактов о SoC** (R3).

## Структура

```
soc/
├── include/abl_target_init.h        # контракт bring-up (abl_target_init/gpio_init)
├── stm32/
│   ├── stm32_target_init.c          # тактирование + SysTick + abl_target_init() (STM32)
│   └── <family>/<variant>/          # SoC-дефиниция (пример: f1/STM32F103x6)
│       ├── soc.cmake                # device, core, fpu, hal_prefix, файлы bring-up
│       ├── startup_*.s
│       ├── system_*.c
│       ├── *_FLASH.ld
│       ├── *.svd
│       └── stm32*xx_hal_conf.h
├── avr/  esp32/                     # (Шаги 8/9)
└── CMakeLists.txt                   # target_init + экспорт SOC_*
```

## Как добавить МК

1. Создать каталог `soc/<family>/<variant>/` с `soc.cmake` и файлами bring-up
   (startup, system, линкер, hal_conf; `.svd` — по желанию).
2. Добавить одну строку в `ABL_SOC_INDEX` (`cmake/helpers/soc.cmake`):
   `"<mcu.part>=<family>/<variant>"`.

Больше ничего: device define, CPU core, FPU/float-ABI, startup/linker/system и
hal_conf подтягиваются автоматически по `mcu.part` из board-конфига.

## Резолвер

`abl_soc_resolve(<part> <prefix>)` из `cmake/helpers/soc.cmake` вызывается:
- в проекте — **до `project()`** (нужно для выбора тулчейна и флагов CPU/FPU),
- внутри `soc/CMakeLists.txt` — для путей bring-up.

Неизвестный `mcu.part` даёт понятную ошибку конфигурации, а не «чудо» на железе.
