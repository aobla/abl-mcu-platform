/**
 * @brief STM32 delay primitives (L1).
 *
 * Both delays are pure busy-waits on the DWT cycle counter, which makes them
 * CPU-frequency accurate and independent of SysTick. That is what keeps
 * abl_delay_* usable when an RTOS owns the tick (R9): sleeping belongs to the
 * runtime contract (abl_sleep_*), not here.
 *
 * Cortex-M0/M0+ have no DWT; the fallback is a naive cycle loop.
 */

#include "abl_delay.h"
#include "soc_hal.h"

#if defined(DWT) && defined(CoreDebug)
#define ABL_DWT_AVAILABLE 1
#else
#define ABL_DWT_AVAILABLE 0
#endif

#if ABL_DWT_AVAILABLE

static void dwt_ensure_running(void)
{
    if ((DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk) == 0U) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0U;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    }
}

/* Wrap-safe: the counter is 32-bit and we only compare deltas. */
static void dwt_delay_cycles(uint32_t cycles)
{
    const uint32_t start = DWT->CYCCNT;
    while ((uint32_t)(DWT->CYCCNT - start) < cycles) {
        /* busy wait */
    }
}

#endif /* ABL_DWT_AVAILABLE */

void abl_delay_us(uint32_t us)
{
#if ABL_DWT_AVAILABLE
    dwt_ensure_running();
    dwt_delay_cycles(us * (SystemCoreClock / 1000000U));
#else
    volatile uint32_t cycles = us * (SystemCoreClock / 1000000U);
    while (cycles-- > 0U) {
        __NOP();
    }
#endif
}

void abl_delay_ms(uint32_t ms)
{
#if ABL_DWT_AVAILABLE
    const uint32_t cycles_per_ms = SystemCoreClock / 1000U;
    /* Keep every wait below 2^31 cycles so the wrap-safe compare stays valid. */
    const uint32_t max_chunk_ms = (cycles_per_ms > 0U) ? (0x7FFFFFFFU / cycles_per_ms) : 0U;

    dwt_ensure_running();

    while (ms > 0U) {
        uint32_t chunk = ms;
        if (max_chunk_ms > 0U && chunk > max_chunk_ms) {
            chunk = max_chunk_ms;
        }
        dwt_delay_cycles(chunk * cycles_per_ms);
        ms -= chunk;
    }
#else
    while (ms-- > 0U) {
        abl_delay_us(1000U);
    }
#endif
}
