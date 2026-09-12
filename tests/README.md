# tests

Хост-тесты на native-таргете (D8): гоняются без железа и без кросс-тулчейнов.

## Запуск

```bash
./scripts/run_host_tests.sh          # configure + build + ctest
```

Скрипт конфигурирует платформу автономно (`-DPLATFORM=native -DABL_BUILD_TESTS=ON`)
и прогоняет `ctest`.

## Что внутри

| Тест | Проверяет |
|---|---|
| `hal_gpio_test` | контракт `abl_gpio` на native-порте: конфигурация, вывод, toggle, симулированный вход, ошибки (NULL, неверный mode, неинициализированный пин) и «прерывания не поддерживаются» |
| `runtime_test` | `abl_sleep_*`, монотонность `abl_uptime_ms`, `abl_task_create` (валидация), вложенность `abl_critical_*` |
| `codegen_test.py` | контракт генераторов: `use:`-ссылки, конфликты пинов, `af/speed/otype`, `CONFIG_*` из фич/параметров |

## Харнесс

`abl_test.h` — минимальный собственный (макросы `ABL_TEST`, `ABL_CHECK`,
`ABL_CHECK_EQ`). Внешние фреймворки сознательно не используются: сборка и тесты
должны работать офлайн, без загрузки зависимостей.

## Особенности

- Тесты линкуют `abl::hal`, `abl::active`, `abl::runtime`, но **не** `abl::soc`:
  иначе `soc/native/native_main.c` принесёт свой `main()`.
- Симуляция портов — через `abl_native_sim.h` (только для тестов; код приложения
  и драйверов этот заголовок не включает, R1).
- Наблюдаемость без отладчика: `ABL_NATIVE_GPIO_TRACE=1 ./build/native/blink-led-native`.
