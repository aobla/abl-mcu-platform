# boards

Board-дефиниции: **физика платы** (что распаяно) + `mcu.part` (D9, ARCHITECTURE §8).

- Один YAML на плату; проект выбирает её по имени: `product.board: <board.name>`.
- Плата описывает только то, что на ней реально есть: МК, кварц, частоты, память,
  распаянное железо (`onboard:` с каноническими алиасами `led0`, `btn0`, ...).
- **Чего здесь быть не должно:** использование пинов конкретным приложением — это
  overlay проекта (`config/<product>.yml`, секция `pins:`, см. `scripts/gen_pinmux.py`).

## Схема

```yaml
board:  { name: bluepill_stm32f103, description: "..." }
platform: stm32f103             # build/toolchain kind (SoC выбирается по mcu.part)
mcu:    { part: STM32F103C6T6 } # единственный ключ → SoC-дефиниция (R3)
cpu:    cortex-m3
oscillator:  { source: hse, freq_hz: 8000000 }
frequencies: { hclk: 72000000, pclk1: 36000000, pclk2: 72000000 }
memory:      { flash: 32768, ram: 10240 }
onboard:
  led0: { port: GPIOC, pin: 13, mode: output, state: low }
```

Поля пина: `port`, `pin`, `mode` (`input|output|alt_function|analog`),
`pull` (`none|up|down`), `state` (`none|high|low`).

## Как добавить плату

1. Скопировать существующий YAML и поправить `board.name`, `mcu.part`, `onboard`.
2. Убедиться, что для `mcu.part` есть SoC-дефиниция (иначе — добавить, см. `soc/README.md`).
3. В проекте указать `product.board: <board.name>`.

## Ограничения текущей версии

- Пины описываются по одному (`usart1_tx`, `usart1_rx`); многопиновые ресурсы
  (USART/SPI одним узлом) — расширение на будущее.
- `oscillator` — пока справочные данные: тактирование задаётся в `soc/.../clock.c`
  (параметризация кварцем из board — задача на будущее).
- AF-номер периферии появится вместе с расширением контракта GPIO (Шаг 8).
