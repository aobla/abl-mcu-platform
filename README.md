# ABL MCU Platform

Кроссплатформенный SDK на открытом ПО для сборки прошивок MCU на Linux (STM32, ESP32, AVR и др.).

> Архитектура, правила и план развития — в [ARCHITECTURE.md](ARCHITECTURE.md), нормативном документе платформы.

## Структура

```
abl-mcu-platform/
├── ARCHITECTURE.md    # конституция платформы
├── LICENSES.md        # инвентарь лицензий (D10)
├── manifest.yml       # vendor-SDK и тулчейны (канонический источник версий)
├── cmake/             # тулчейны и CMake-хелперы (abl_component, резолвер SoC)
├── hal/               # контракты (include/) + порты (src/{stm32,avr,esp32,native})
├── runtime/           # контракт рантайма + бэкенды bare / freertos
├── soc/               # bring-up по SoC (тактирование, startup, линкер)
├── boards/            # board-дефиниции (YAML)
├── drivers/           # переносимые драйверы
├── system/            # логирование, CLI (по мере роста)
├── connect/           # сетевые протоколы (будущее)
├── templates/         # шаблоны кодогенерации
├── scripts/           # кодогенерация и утилиты
└── tests/             # хост-тесты (native)
```

## HAL

Унифицированный интерфейс к периферии:

```c
#include "abl_gpio.h"

abl_gpio_pin_t led = { .port = LED_PORT, .pin = LED_PIN };
abl_gpio_init(&led, ABL_GPIO_MODE_OUTPUT, ABL_GPIO_PULL_NONE, ABL_GPIO_STATE_NONE);
abl_gpio_write(&led, true);
```

## Использование

Подключается к проекту через `lib/abl-mcu-platform` (наполняется `setup.sh -d` из манифеста). Шаблон приложения — `abl-mcu-project-*`.

## Лицензия

MIT License. Vendor-компоненты сохраняют свои лицензии (см. LICENSES.md).
