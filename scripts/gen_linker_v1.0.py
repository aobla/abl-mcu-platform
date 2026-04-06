#!/usr/bin/env python3
"""
Скрипт генерации линкер-скрипта из YAML файла
"""

import yaml
import sys
import os
from pathlib import Path
from jinja2 import Template

def generate_linker_script(config_data, output_dir):
    """Генерирует linker_script.ld из YAML-файла"""

    # Создаем директорию для генерации, если не существует
    generated_dir = Path(output_dir) / "generated"
    generated_dir.mkdir(parents=True, exist_ok=True)

    # Шаблон линкер-скрипта
    linker_template = """/* Generated from YAML configuration */
/* Linker script for {{ config.platform }} */

{% set flash_size = config.memory.flash %}
{% set ram_size = config.memory.ram %}

MEMORY
{
{% if config.platform.startswith('stm32') %}
  /* STM32 Memory Layout */
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = {{ flash_size }}
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = {{ ram_size }}
{% elif config.platform.startswith('esp32') %}
  /* ESP32 Memory Layout */
  FLASH (rx)      : ORIGIN = 0x10000000, LENGTH = {{ flash_size }}
  RAM (rwx)       : ORIGIN = 0x3FFC0000, LENGTH = {{ ram_size }}
{% elif config.platform.startswith('avr') %}
  /* AVR Memory Layout */
  FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = {{ flash_size }}
  RAM (rwx)       : ORIGIN = 0x00800100, LENGTH = {{ ram_size }}
{% else %}
  /* Generic Memory Layout */
  FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = {{ flash_size }}
  RAM (rwx)       : ORIGIN = 0x20000000, LENGTH = {{ ram_size }}
{% endif %}
}

/* Entry point — platform-specific */
{% if config.platform.startswith('stm32') %}
ENTRY(Reset_Handler)
{% elif config.platform.startswith('avr') %}
ENTRY(__vector_reset)
{% elif config.platform.startswith('esp32') %}
ENTRY(call_start_cpu0)
{% else %}
ENTRY(Reset_Handler)
{% endif %}

/* Sections */
SECTIONS
{
{% if config.platform.startswith('avr') %}
  /* AVR sections — используем стандартные имена */
  .text :
  {
    *(.vectors)
    *(.text*)
    *(.rodata*)
    _etext = .;
  } > FLASH

  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH

  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM
{% else %}
  /* Vector table and code */
  .text :
  {
    KEEP(*(.isr_vector))
    *(.text*)
    *(.rodata*)
    _etext = .;
  } > FLASH

  /* Initialized data */
  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH

  /* Uninitialized data */
  .bss :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  /* Stack section */
  ._stack :
  {
    _estack = ORIGIN(RAM) + LENGTH(RAM);
  } > RAM
{% endif %}
}
"""

    linker_template_obj = Template(linker_template)
    linker_content = linker_template_obj.render(config=config_data)

    linker_script_path = generated_dir / "linker_script.ld"
    with open(linker_script_path, 'w', encoding='utf-8') as f:
        f.write(linker_content)

    print(f"Generated {linker_script_path}")

def validate_config(config):
    """Проверяет корректность конфигурации для линкер-скрипта"""
    errors = []

    # Проверяем память
    memory = config.get('memory', {})
    if not isinstance(memory, dict):
        errors.append("'memory' should be a dictionary")
        return errors

    if 'flash' not in memory:
        errors.append("Missing 'memory.flash' field")
    elif not isinstance(memory['flash'], int) or memory['flash'] <= 0:
        errors.append("'memory.flash' should be a positive integer")

    if 'ram' not in memory:
        errors.append("Missing 'memory.ram' field")
    elif not isinstance(memory['ram'], int) or memory['ram'] <= 0:
        errors.append("'memory.ram' should be a positive integer")

    return errors

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_linker_v1.0.py <config_yaml_path>")
        sys.exit(1)

    yaml_path = sys.argv[1]

    # Проверяем существование файла
    if not os.path.exists(yaml_path):
        print(f"Error: Config file {yaml_path} does not exist")
        sys.exit(1)

    try:
        # Загружаем и валидируем конфиг
        with open(yaml_path, 'r', encoding='utf-8') as f:
            config = yaml.safe_load(f)

        errors = validate_config(config)
        if errors:
            print("Linker configuration validation errors:")
            for error in errors:
                print(f"  - {error}")
            sys.exit(1)

        # Генерируем линкер-скрипт
        output_dir = os.getcwd()  # Используем текущую директорию как output
        generate_linker_script(config, output_dir)

    except yaml.YAMLError as e:
        print(f"Error parsing YAML file: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error generating linker script: {e}")
        sys.exit(1)
