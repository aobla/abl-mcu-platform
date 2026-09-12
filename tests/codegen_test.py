#!/usr/bin/env python3
"""
Code generation contract tests (host, no hardware) — ARCHITECTURE §16.

    codegen_test.py [platform-root]

Runs the real generators on temporary board/app fixtures and checks:
  1. a valid pair generates the expected macros and handles;
  2. a board pin field (af/speed/otype) survives into the generated header;
  3. app features/params become CONFIG_* macros;
  4. an unknown `use:` reference fails loudly;
  5. two logical pins on one physical pin fail loudly.
"""

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent
GENERATE = ROOT / "scripts" / "generate.py"

BOARD = """
board: { name: fixture, description: "codegen fixture" }
platform: native
mcu: { part: HOST }
cpu: x86_64
frequencies: { cpu: 1000000 }
memory: { flash: 0, ram: 0 }
onboard:
  led0: { port: 0, pin: 3, mode: output, state: low }
  btn0: { port: 0, pin: 4, mode: input, pull: up, af: 7, speed: high, otype: open_drain }
"""

APP_OK = """
product: { id: fixture, board: fixture }
pins:
  led:    { use: led0 }
  button: { use: btn0 }
features: { enable_log: true }
params:   { blink_period_ms: 42 }
"""

APP_BAD_USE = """
product: { id: fixture, board: fixture }
pins:
  led: { use: nope }
"""

APP_CONFLICT = """
product: { id: fixture, board: fixture }
pins:
  a: { use: led0 }
  b: { port: 0, pin: 3 }
"""

failures = 0


def check(condition, message):
    global failures
    if condition:
        print(f"  [ok]   {message}")
    else:
        failures += 1
        print(f"  [FAIL] {message}")


def run(board_text, app_text):
    with tempfile.TemporaryDirectory() as tmp:
        tmp = Path(tmp)
        board = tmp / "board.yml"
        app = tmp / "app.yml"
        board.write_text(board_text, encoding="utf-8")
        app.write_text(app_text, encoding="utf-8")

        proc = subprocess.run(
            [sys.executable, str(GENERATE), str(board), str(app)],
            cwd=tmp, capture_output=True, text=True,
        )

        generated = {}
        for name in ("hardware_config.h", "hardware_pins.h", "generated_gpio_init.c"):
            path = tmp / "generated" / name
            generated[name] = path.read_text(encoding="utf-8") if path.is_file() else ""
        return proc, generated


def main():
    print("codegen contract tests")

    proc, gen = run(BOARD, APP_OK)
    check(proc.returncode == 0, "valid board + app generate successfully")
    pins = gen["hardware_pins.h"]
    check("#define LED_PORT  ((void*)(uintptr_t)0)" in pins, "numeric port is encoded as a pointer")
    check("#define LED_PIN   3" in pins, "pin number is emitted")
    check("#define BUTTON_AF    7" in pins, "board af survives into the header")
    check("#define BUTTON_SPEED ABL_GPIO_SPEED_HIGH" in pins, "board speed survives")
    check("#define BUTTON_OTYPE ABL_GPIO_OTYPE_OPEN_DRAIN" in pins, "board otype survives")
    check("abl_gpio_configure(&gpio_init_table[i].pin" in gen["generated_gpio_init.c"],
          "generated init uses abl_gpio_configure()")

    config = gen["hardware_config.h"]
    check("#define CONFIG_BLINK_PERIOD_MS 42" in config, "app params become CONFIG_* macros")
    check("#define CONFIG_ENABLE_LOG 1" in config, "app features become CONFIG_* macros")

    proc, _ = run(BOARD, APP_BAD_USE)
    check(proc.returncode != 0 and "not an onboard resource" in proc.stderr,
          "unknown use: reference fails loudly")

    proc, _ = run(BOARD, APP_CONFLICT)
    check(proc.returncode != 0 and "pin conflict" in proc.stderr,
          "two pins on one physical pin fail loudly")

    print(f"{failures} failures")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
