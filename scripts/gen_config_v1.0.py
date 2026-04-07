#!/usr/bin/env python3
"""
Скрипт генерации конфигурации из YAML файла
"""

import yaml
import sys
import os
from pathlib import Path
from jinja2 import Template

def generate_hardware_config(config_data, output_dir):
    """Генерирует hardware_config.h из YAML-файла"""
    
    # Создаем директорию для генерации, если не существует
    generated_dir = Path(output_dir) / "generated"
    generated_dir.mkdir(parents=True, exist_ok=True)
    
    # Исключаем секцию pins из конфигурации
    platform_config = {}
    for key, value in config_data.items():
        if key != 'pins':
            platform_config[key] = value
    
    # Генерируем hardware_config.h
    config_template = """#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

/* Generated from YAML configuration */

{% for key, value in config.items() %}
{% if key == 'platform' %}
#ifndef PLATFORM_{{ value.upper().replace('-', '_') }}
#define PLATFORM_{{ value.upper().replace('-', '_') }}
#endif
{% elif key == 'cpu' %}
#ifndef CPU_{{ value.upper().replace('-', '_').replace('.', '_') }}
#define CPU_{{ value.upper().replace('-', '_').replace('.', '_') }}
#endif
{% elif key == 'frequencies' %}
{% for freq_name, freq_value in value.items() %}
#ifndef {{ freq_name.upper() }}_FREQ
#define {{ freq_name.upper() }}_FREQ {{ freq_value }}
#endif
{% endfor %}
{% elif key == 'memory' %}
{% for mem_type, mem_value in value.items() %}
#ifndef {{ mem_type.upper() }}_SIZE
#define {{ mem_type.upper() }}_SIZE {{ mem_value }}
#endif
{% endfor %}
{% elif key == 'features' %}
{% for feature_name, feature_enabled in value.items() %}
{% set clean_name = feature_name.upper().replace('ENABLE_', '') %}
#ifndef CONFIG_{{ clean_name }}
#define CONFIG_{{ clean_name }} {% if feature_enabled %}1{% else %}0{% endif %}
#endif
{% endfor %}
{% endif %}
{% endfor %}

#endif // HARDWARE_CONFIG_H
"""
    
    config_template_obj = Template(config_template)
    config_content = config_template_obj.render(config=platform_config)
    
    config_header_path = generated_dir / "hardware_config.h"
    with open(config_header_path, 'w', encoding='utf-8') as f:
        f.write(config_content)
    
    print(f"Generated {config_header_path}")

def validate_config(config):
    """Проверяет корректность конфигурации"""
    errors = []
    
    # Проверяем обязательные поля
    if 'platform' not in config:
        errors.append("Missing 'platform' field in config")
    
    if 'cpu' not in config:
        errors.append("Missing 'cpu' field in config")
    
    # Проверяем частоты
    frequencies = config.get('frequencies', {})
    if not isinstance(frequencies, dict):
        errors.append("'frequencies' should be a dictionary")
    
    # Проверяем память
    memory = config.get('memory', {})
    if not isinstance(memory, dict):
        errors.append("'memory' should be a dictionary")
    
    # Проверяем фичи
    features = config.get('features', {})
    if not isinstance(features, dict):
        errors.append("'features' should be a dictionary")
    
    return errors

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 gen_config_v1.0.py <config_yaml_path>")
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
            print("Configuration validation errors:")
            for error in errors:
                print(f"  - {error}")
            sys.exit(1)
        
        # Генерируем конфигурационный файл
        output_dir = os.getcwd()  # Используем текущую директорию как output
        generate_hardware_config(config, output_dir)
        
    except yaml.YAMLError as e:
        print(f"Error parsing YAML file: {e}")
        sys.exit(1)
    except Exception as e:
        print(f"Error generating config: {e}")
        sys.exit(1)