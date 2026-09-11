/**
 * @brief Общий bring-up STM32 — без привязки к конкретному SoC.
 *
 * Конкретика приходит из SoC-дефиниции (soc/<family>/<variant>/):
 *   - "soc_hal.h"        — vendor-заголовок семейства;
 *   - soc_clock_init()   — тактирование этого SoC (clock.c).
 * Ни одного #ifdef PLATFORM_* здесь нет: новый МК добавляется без правки этого файла.
 */

#include "abl_app.h"
#include "abl_target_init.h"
#include "soc_hal.h"
#include "soc_clock.h"

/**
 * @brief SysTick handler — required for HAL_Delay()
 *
 * HAL_Init() enables SysTick, but the startup file maps SysTick_Handler
 * to Default_Handler (infinite loop). This override increments uwTick
 * so HAL_Delay() actually works.
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

void abl_target_init(void)
{
    HAL_Init();
    soc_clock_init();
    abl_target_gpio_init();
}

void abl_target_gpio_init(void)
{
    generated_gpio_init();
}

/**
 * @brief Платформенный трaмполин (D7).
 *
 * Startup вызывает main(); приложение живёт в переносимой abl_main().
 */
int main(void)
{
    abl_main();
    return 0;
}
