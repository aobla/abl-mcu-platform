# ABL MCU Platform Core 

Это ядро кроссплатформенной MCU-платформы, обеспечивающее унифицированный доступ к периферии микроконтроллеров различных архитектур (STM32, ESP32, AVR).

## Архитектура

Платформа состоит из 4 репозиториев:
- `abl-mcu-platform-core` - ядро с базовым HAL и тулчейнами
- `abl-mcu-platform-system` - системы уровня ОС (RTOS, логиров  ание, CLI)
- `abl-mcu-platform-connect` - сетевые протоколы (HTTP, MQTT, TCP/IP)
- `abl-mcu-platform-drivers` - драйверы периферии (сенсоры, дисплеи)

## Структура

```
abl-mcu-platform-core/
├── CMakeLists.txt              # Точка входа, проверка версий
├── cmake/
│   ├── toolchains/             # Тулчейны для разных архитектур
│   │   ├── stm32-gcc.cmake
│   │   ├── esp32-clang.cmake
│   │   └── avr-gcc.cmake
│   └── helpers/                # CMake хелперы
│       ├── target_helpers.cmake
│       └── version_check.cmake
├── scripts/                    # Скрипты генерации
│   ├── gen_config_v1.0.py
│   └── gen_pinmux_v1.0.py
├── hal/                        # Hardware Abstraction Layer
│   ├── include/hal/
│   │   ├── gpio.h
│   │   └── ...
│   └── src/
│       ├── stm32/
│       ├── esp32/
│       └── avr/
├── cpp_wrappers/               # C++ обертки (опционально)
├── templates/                  # Шаблоны для генерации
└── tests/                      # Тесты
```

## HAL (Hardware Abstraction Layer)

HAL предоставляет унифицированный интерфейс для работы с периферией:

```c
#include "hal/gpio.h"

hal_gpio_pin_t led_pin = { .port = LED_PORT, .pin = LED_PIN };
hal_gpio_init(&led_pin, HAL_GPIO_MODE_OUTPUT, HAL_GPIO_PULL_NONE);
hal_gpio_write(&led_pin, true);
```

## Генерация конфигурации

Конфигурация проекта задается в YAML-файлах и автоматически генерирует заголовочные файлы:

- `board_config.yml` - конфигурация платы (частоты, память, фичи)
- `board_pinmux.yml` - конфигурация пинов

## Поддерживаемые архитектуры

- **STM32**: Все серии (F1, F4, F7, H7 и др.)
- **ESP32**: Все модели (ESP32, ESP32-S, ESP32-C и др.)
- **AVR**: ATmega (ATmega328P, ATmega2560 и др.)

## Использование

Для использования в проекте добавьте зависимость через CMake FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    platformcore
    GIT_REPOSITORY https://github.com/myorg/abl-mcu-platform-core.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(platformcore)
```

## Сборка

Для сборки под разные платформы используйте CMake presets:

```bash
# Для STM32F4
cmake --preset=stm32f4-release
cmake --build --preset=stm32f4-release-build

# Для ESP32
cmake --preset=esp32-release
cmake --build --preset=esp32-release-build
```

## Лицензия

MIT License