#!/usr/bin/env python3
"""
Скрипт генерации пинмапа из YAML файла
"""

import yaml
import sys
import os
from pathlib import Path
from jinja2 import Template

def generate_pinmux_config(yaml_path, output_dir):
    """Генерирует hardware_pins.h и generated_gpio_init.c из YAML-файла"""
    
    # Загружаем YAML-файл
    with open(yaml_path, 'r', encoding='utf-8') as f:
        config = yaml.safe_load(f)
    
    # Получаем только секцию pins
    pins = config.get('pins', {})
    
    # Создаем директорию для генерации, если не существует
    generated_dir = Path(output_dir) / "generated"
    generated_dir.mkdir(parents=True, exist_ok=True)
    
    # Генерируем hardware_pins.h
    pins_template = """#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#include "abl_common.h"

/* Generated from YAML pin configuration */

/* ─── Pin definitions (port, pin number) ──────────────────────────────────── */
{% for pin_name, pin_config in pins.items() %}
{% set port_val = pin_config.port %}
{% if port_val is number %}
#define {{ pin_name.upper() }}_PORT ((void*)(uintptr_t){{ port_val }})
{% else %}
#define {{ pin_name.upper() }}_PORT {{ port_val }}
{% endif %}
#define {{ pin_name.upper() }}_PIN {{ pin_config.pin }}
#define {{ pin_name.upper() }}_PULL {% if pin_config.pull %}{{ pin_config.pull.upper() }}{% else %}NONE{% endif %}
#define {{ pin_name.upper() }}_STATE {% if pin_config.state %}{{ pin_config.state.upper() }}{% else %}NONE{% endif %}
{% endfor %}

/* ─── Static const pin handles (stored in FLASH, zero RAM) ────────────────── */
{% for pin_name, pin_config in pins.items() %}
static const abl_gpio_pin_t pin_{{ pin_name.lower() }} = { {{ pin_name.upper() }}_PORT, {{ pin_name.upper() }}_PIN };
{% endfor %}

/* ─── Convenience macros ──────────────────────────────────────────────────── */
/** Get pointer to a pin handle by name (lowercase) */
#define PIN_GET(name) (&pin_##name)

/** Toggle a pin by name */
#define abl_gpio_toggle_pin(name)  abl_gpio_toggle(PIN_GET(name))

/** Write a pin by name */
#define abl_gpio_write_pin(name, state)  abl_gpio_write(PIN_GET(name), state)

/** Read a pin by name */
#define abl_gpio_read_pin(name, out)  abl_gpio_read(PIN_GET(name), out)

#endif // HARDWARE_PINS_H
"""
    
    pins_template_obj = Template(pins_template)
    pins_content = pins_template_obj.render(pins=pins)
    
    pins_header_path = generated_dir / "hardware_pins.h"
    with open(pins_header_path, 'w', encoding='utf-8') as f:
        f.write(pins_content)
    
    print(f"Generated {pins_header_path}")
    
    # Генерируем generated_gpio_init.c с функцией инициализации GPIO
    # Оптимизация: const таблица во FLASH + цикл, вместо локальных переменных на стеке
    gpio_init_template = """#include "app.h"

/**
 * @brief Таблица инициализации GPIO (хранится во FLASH, не тратит RAM)
 */
typedef struct {
    void*         port;
    uint16_t      pin;
    abl_gpio_mode_t mode;
    abl_gpio_pull_t pull;
    abl_gpio_state_t state;
} gpio_init_entry_t;

static const gpio_init_entry_t gpio_init_table[] = {
{% for pin_name, pin_config in pins.items() %}
    { {{ pin_name.upper() }}_PORT, {{ pin_name.upper() }}_PIN,
      {% if pin_config.mode == 'input' %}ABL_GPIO_MODE_INPUT{% elif pin_config.mode == 'alt_function' %}ABL_GPIO_MODE_ALT_FUNCTION{% elif pin_config.mode == 'analog' %}ABL_GPIO_MODE_ANALOG{% else %}ABL_GPIO_MODE_OUTPUT{% endif %},
      {% if pin_config.pull == 'up' %}ABL_GPIO_PULL_UP{% elif pin_config.pull == 'down' %}ABL_GPIO_PULL_DOWN{% else %}ABL_GPIO_PULL_NONE{% endif %},
      {% if pin_config.state == 'high' %}ABL_GPIO_STATE_HIGH{% elif pin_config.state == 'low' %}ABL_GPIO_STATE_LOW{% else %}ABL_GPIO_STATE_NONE{% endif %} },
{% endfor %}
};

/**
 * @brief Сгенерированная функция инициализации GPIO пинов
 */
void generated_gpio_init(void) {
    for (size_t i = 0; i < sizeof(gpio_init_table) / sizeof(gpio_init_table[0]); i++) {
        abl_gpio_pin_t pin = { gpio_init_table[i].port, gpio_init_table[i].pin };
        abl_gpio_init(&pin, gpio_init_table[i].mode, gpio_init_table[i].pull, gpio_init_table[i].state);
    }
}
"""
    
    gpio_init_template_obj = Template(gpio_init_template)
    gpio_init_content = gpio_init_template_obj.render(pins=pins)
    
    gpio_init_path = generated_dir / "generated_gpio_init.c"
    with open(gpio_init_path, 'w', encoding='utf-8') as f:
        f.write(gpio_init_content)
    
    print(f"Generated {gpio_init_path}")

def validate_pinmux_config(config):
    """Проверяет корректность конфигурации пинов"""
    errors = []

    pins = config.get('pins', {})
    if not isinstance(pins, dict):
        errors.append("'pins' should be a dictionary")
        return errors

    # Проверяем на конфликты пинов (один пин в разных ролях)
    seen_pins = {}
    for pin_name, pin_config in pins.items():
        if not isinstance(pin_config, dict):
            errors.append(f"Pin '{pin_name}' configuration should be a dictionary")
            continue

        if 'port' not in pin_config:
            errors.append(f"Pin '{pin_name}' missing 'port' field")
            continue

        if 'pin' not in pin_config:
            errors.append(f"Pin '{pin_name}' missing 'pin' field")
            continue
        
        pin_value = pin_config['pin']
        if not isinstance(pin_value, int) or pin_value < 0:
            errors.append(f"Pin '{pin_name}' has invalid pin number: {pin_value}")
            continue

        # Проверяем конфликт пинов
        pin_key = (pin_config['port'], pin_value)
        if pin_key in seen_pins:
            errors.append(f"Pin conflict: '{pin_name}' and '{seen_pins[pin_key]}' both use port={pin_config['port']}, pin={pin_value}")
        else:
            seen_pins[pin_key] = pin_name

        if 'mode' not in pin_config:
            errors.append(f"Pin '{pin_name}' missing 'mode' field")
        else:
            valid_modes = ['input', 'output', 'alt_function', 'analog']
            if pin_config['mode'] not in valid_modes:
                errors.append(f"Pin '{pin_name}' has invalid mode: {pin_config['mode']}")

    return errors

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_pinmux_v1.0.py <pinmux_yaml_path>")
        sys.exit(1)
    
    yaml_path = sys.argv[1]
    
    # Проверяем существование файла
    if not os.path.exists(yaml_path):
        print(f"Error: Pinmux file {yaml_path} does not exist")
        sys.exit(1)
    
    try:
        # Загружаем и валидируем конфиг
        with open(yaml_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f)
        
        errors = validate_pinmux_config(config)
        if errors:
            print("Pinmux configuration validation errors:")
            for error in errors:
                print(f"  - {error}")
            sys.exit(1)
        
        # Генерируем конфигурационный файл
        output_dir = os.getcwd()  # Используем текущую директорию как output
        generate_pinmux_config(yaml_path, output_dir)
        
    except yaml.YAMLError as e:
        print(f"Error parsing YAML file: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error generating pinmux: {e}")
        sys.exit(1)