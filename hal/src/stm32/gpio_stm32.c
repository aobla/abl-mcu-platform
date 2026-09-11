/**
 * @brief STM32 GPIO port (L1).
 *
 * Implements the abl_gpio contract on top of the vendor HAL:
 *   - full configuration (mode/pull/state/speed/output type/AF number);
 *   - per-pin interrupts routed through the HAL EXTI module, which is the
 *     family-agnostic way to reach EXTI on F1/F4/H7 (register naming differs).
 *
 * The AF number is only applied on families that have the AFR registers
 * (F1 selects alternate functions through CNF/MODE bits instead).
 */

#include "abl_gpio.h"
#include "soc_hal.h"

/* ─── Helpers ───────────────────────────────────────────────────────────── */

static bool gpio_clk_enable(GPIO_TypeDef* port)
{
#if defined(GPIOA)
    if (port == GPIOA) { __HAL_RCC_GPIOA_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOB)
    if (port == GPIOB) { __HAL_RCC_GPIOB_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOC)
    if (port == GPIOC) { __HAL_RCC_GPIOC_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOD)
    if (port == GPIOD) { __HAL_RCC_GPIOD_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOE)
    if (port == GPIOE) { __HAL_RCC_GPIOE_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOF)
    if (port == GPIOF) { __HAL_RCC_GPIOF_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOG)
    if (port == GPIOG) { __HAL_RCC_GPIOG_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOH)
    if (port == GPIOH) { __HAL_RCC_GPIOH_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOI)
    if (port == GPIOI) { __HAL_RCC_GPIOI_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOJ)
    if (port == GPIOJ) { __HAL_RCC_GPIOJ_CLK_ENABLE(); return true; }
#endif
#if defined(GPIOK)
    if (port == GPIOK) { __HAL_RCC_GPIOK_CLK_ENABLE(); return true; }
#endif
    return false;
}

static uint32_t pull_to_hal(abl_gpio_pull_t pull)
{
    switch (pull) {
        case ABL_GPIO_PULL_UP:   return GPIO_PULLUP;
        case ABL_GPIO_PULL_DOWN: return GPIO_PULLDOWN;
        default:                 return GPIO_NOPULL;
    }
}

static uint32_t speed_to_hal(abl_gpio_speed_t speed)
{
    switch (speed) {
        case ABL_GPIO_SPEED_MEDIUM: return GPIO_SPEED_FREQ_MEDIUM;
        case ABL_GPIO_SPEED_HIGH:   return GPIO_SPEED_FREQ_HIGH;
#if defined(GPIO_SPEED_FREQ_VERY_HIGH)
        case ABL_GPIO_SPEED_VERY_HIGH: return GPIO_SPEED_FREQ_VERY_HIGH;
#endif
        default: return GPIO_SPEED_FREQ_LOW;
    }
}

/* ─── Configuration ─────────────────────────────────────────────────────── */

abl_status_t abl_gpio_configure(const abl_gpio_pin_t* pin, const abl_gpio_cfg_t* cfg)
{
    if (pin == 0 || pin->port == 0 || cfg == 0 || pin->pin > 15U) {
        return ABL_STATUS_ERROR;
    }

    GPIO_TypeDef* port = (GPIO_TypeDef*)pin->port;
    if (!gpio_clk_enable(port)) {
        return ABL_STATUS_ERROR;
    }

    GPIO_InitTypeDef init = {0};
    init.Pin   = (uint32_t)(1UL << pin->pin);
    init.Pull  = pull_to_hal(cfg->pull);
    init.Speed = speed_to_hal(cfg->speed);

    switch (cfg->mode) {
        case ABL_GPIO_MODE_INPUT:
            init.Mode = GPIO_MODE_INPUT;
            break;

        case ABL_GPIO_MODE_OUTPUT:
            init.Mode = (cfg->otype == ABL_GPIO_OTYPE_OPEN_DRAIN) ? GPIO_MODE_OUTPUT_OD
                                                                 : GPIO_MODE_OUTPUT_PP;
            break;

        case ABL_GPIO_MODE_ALT_FUNCTION:
            init.Mode = (cfg->otype == ABL_GPIO_OTYPE_OPEN_DRAIN) ? GPIO_MODE_AF_OD
                                                                 : GPIO_MODE_AF_PP;
#if defined(GPIO_AFRL) || defined(GPIO_AFRH)
            init.Alternate = cfg->af;
#endif
            break;

        case ABL_GPIO_MODE_ANALOG:
            init.Mode = GPIO_MODE_ANALOG;
            break;

        default:
            return ABL_STATUS_ERROR;
    }

    HAL_GPIO_Init(port, &init);

    if (cfg->mode == ABL_GPIO_MODE_OUTPUT && cfg->state != ABL_GPIO_STATE_NONE) {
        HAL_GPIO_WritePin(port, init.Pin,
                          (cfg->state == ABL_GPIO_STATE_HIGH) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_init(const abl_gpio_pin_t* pin, abl_gpio_mode_t mode,
                           abl_gpio_pull_t pull, abl_gpio_state_t state)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = mode;
    cfg.pull  = pull;
    cfg.state = state;
    return abl_gpio_configure(pin, &cfg);
}

/* ─── Basic I/O ─────────────────────────────────────────────────────────── */

abl_status_t abl_gpio_write(const abl_gpio_pin_t* pin, bool state)
{
    if (pin == 0 || pin->port == 0 || pin->pin > 15U) {
        return ABL_STATUS_ERROR;
    }

    HAL_GPIO_WritePin((GPIO_TypeDef*)pin->port, (uint16_t)(1UL << pin->pin),
                      state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_toggle(const abl_gpio_pin_t* pin)
{
    if (pin == 0 || pin->port == 0 || pin->pin > 15U) {
        return ABL_STATUS_ERROR;
    }

    HAL_GPIO_TogglePin((GPIO_TypeDef*)pin->port, (uint16_t)(1UL << pin->pin));
    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_read(const abl_gpio_pin_t* pin, bool* state)
{
    if (pin == 0 || pin->port == 0 || state == 0 || pin->pin > 15U) {
        return ABL_STATUS_ERROR;
    }

    *state = (HAL_GPIO_ReadPin((GPIO_TypeDef*)pin->port, (uint16_t)(1UL << pin->pin)) == GPIO_PIN_SET);
    return ABL_STATUS_OK;
}

/* ─── Interrupts (EXTI) ─────────────────────────────────────────────────── */

#if defined(HAL_EXTI_MODULE_ENABLED)

#define ABL_GPIO_IRQ_LINES 16U

typedef struct {
    abl_gpio_irq_cb_t cb;
    void*             arg;
    GPIO_TypeDef*     port;   /* an EXTI line is driven by one port only */
    bool              used;
} abl_gpio_irq_slot_t;

static abl_gpio_irq_slot_t s_irq[ABL_GPIO_IRQ_LINES];
static EXTI_HandleTypeDef  s_exti[ABL_GPIO_IRQ_LINES];

static const uint32_t s_exti_line[ABL_GPIO_IRQ_LINES] = {
    EXTI_LINE_0,  EXTI_LINE_1,  EXTI_LINE_2,  EXTI_LINE_3,
    EXTI_LINE_4,  EXTI_LINE_5,  EXTI_LINE_6,  EXTI_LINE_7,
    EXTI_LINE_8,  EXTI_LINE_9,  EXTI_LINE_10, EXTI_LINE_11,
    EXTI_LINE_12, EXTI_LINE_13, EXTI_LINE_14, EXTI_LINE_15,
};

static void exti_notify(uint32_t line)
{
    if (s_irq[line].used && s_irq[line].cb != 0) {
        abl_gpio_pin_t pin = { s_irq[line].port, (uint16_t)line };
        s_irq[line].cb(&pin, s_irq[line].arg);
    }
}

/* HAL EXTI callbacks take no argument, so every line gets a trampoline. */
#define ABL_EXTI_TRAMPOLINE(n) static void exti_cb_##n(void) { exti_notify(n); }
ABL_EXTI_TRAMPOLINE(0)  ABL_EXTI_TRAMPOLINE(1)  ABL_EXTI_TRAMPOLINE(2)  ABL_EXTI_TRAMPOLINE(3)
ABL_EXTI_TRAMPOLINE(4)  ABL_EXTI_TRAMPOLINE(5)  ABL_EXTI_TRAMPOLINE(6)  ABL_EXTI_TRAMPOLINE(7)
ABL_EXTI_TRAMPOLINE(8)  ABL_EXTI_TRAMPOLINE(9)  ABL_EXTI_TRAMPOLINE(10) ABL_EXTI_TRAMPOLINE(11)
ABL_EXTI_TRAMPOLINE(12) ABL_EXTI_TRAMPOLINE(13) ABL_EXTI_TRAMPOLINE(14) ABL_EXTI_TRAMPOLINE(15)

static void (*const s_exti_cb[ABL_GPIO_IRQ_LINES])(void) = {
    exti_cb_0,  exti_cb_1,  exti_cb_2,  exti_cb_3,
    exti_cb_4,  exti_cb_5,  exti_cb_6,  exti_cb_7,
    exti_cb_8,  exti_cb_9,  exti_cb_10, exti_cb_11,
    exti_cb_12, exti_cb_13, exti_cb_14, exti_cb_15,
};

#define ABL_EXTI_SEL(PORT) \
    do { if (port == PORT) { return EXTI_##PORT; } } while (0)

static uint32_t exti_gpiosel(GPIO_TypeDef* port)
{
#if defined(GPIOA)
    ABL_EXTI_SEL(GPIOA);
#endif
#if defined(GPIOB)
    ABL_EXTI_SEL(GPIOB);
#endif
#if defined(GPIOC)
    ABL_EXTI_SEL(GPIOC);
#endif
#if defined(GPIOD)
    ABL_EXTI_SEL(GPIOD);
#endif
#if defined(GPIOE)
    ABL_EXTI_SEL(GPIOE);
#endif
#if defined(GPIOF)
    ABL_EXTI_SEL(GPIOF);
#endif
#if defined(GPIOG)
    ABL_EXTI_SEL(GPIOG);
#endif
#if defined(GPIOH)
    ABL_EXTI_SEL(GPIOH);
#endif
#if defined(GPIOI)
    ABL_EXTI_SEL(GPIOI);
#endif
    return 0xFFFFFFFFu;
}

static IRQn_Type exti_irqn(uint32_t line)
{
    if (line < 5U)  { return (IRQn_Type)((uint32_t)EXTI0_IRQn + line); }
    if (line < 10U) { return EXTI9_5_IRQn; }
    return EXTI15_10_IRQn;
}

abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg)
{
    if (pin == 0 || pin->port == 0 || cb == 0 || pin->pin >= ABL_GPIO_IRQ_LINES) {
        return ABL_STATUS_ERROR;
    }

    const uint32_t line = pin->pin;
    GPIO_TypeDef*  port = (GPIO_TypeDef*)pin->port;
    const uint32_t sel  = exti_gpiosel(port);

    if (sel == 0xFFFFFFFFu) {
        return ABL_STATUS_ERROR;
    }
    /* Hardware limitation: one port per EXTI line. */
    if (s_irq[line].used && s_irq[line].port != port) {
        return ABL_STATUS_ERROR;
    }

    uint32_t hal_trigger;
    switch (trigger) {
        case ABL_GPIO_IRQ_RISING:  hal_trigger = EXTI_TRIGGER_RISING; break;
        case ABL_GPIO_IRQ_FALLING: hal_trigger = EXTI_TRIGGER_FALLING; break;
        case ABL_GPIO_IRQ_BOTH:    hal_trigger = EXTI_TRIGGER_RISING_FALLING; break;
        default:                   return ABL_STATUS_ERROR;
    }

    if (HAL_EXTI_GetHandle(&s_exti[line], s_exti_line[line]) != HAL_OK) {
        return ABL_STATUS_ERROR;
    }

    EXTI_ConfigTypeDef cfg = {0};
    cfg.Line    = s_exti_line[line];
    cfg.Mode    = EXTI_MODE_INTERRUPT;
    cfg.Trigger = hal_trigger;
    cfg.GPIOSel = sel;

    if (HAL_EXTI_SetConfigLine(&s_exti[line], &cfg) != HAL_OK) {
        return ABL_STATUS_ERROR;
    }
    if (HAL_EXTI_RegisterCallback(&s_exti[line], HAL_EXTI_COMMON_CB_ID,
                                  s_exti_cb[line]) != HAL_OK) {
        return ABL_STATUS_ERROR;
    }

    s_irq[line].cb   = cb;
    s_irq[line].arg  = arg;
    s_irq[line].port = port;
    s_irq[line].used = true;

    HAL_NVIC_SetPriority(exti_irqn(line), 5U, 0U);
    HAL_NVIC_EnableIRQ(exti_irqn(line));

    return ABL_STATUS_OK;
}

abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin)
{
    if (pin == 0 || pin->pin >= ABL_GPIO_IRQ_LINES) {
        return ABL_STATUS_ERROR;
    }

    const uint32_t line = pin->pin;
    if (!s_irq[line].used) {
        return ABL_STATUS_OK;
    }

    HAL_NVIC_DisableIRQ(exti_irqn(line));
    (void)HAL_EXTI_ClearConfigLine(&s_exti[line]);

    s_irq[line].cb   = 0;
    s_irq[line].arg  = 0;
    s_irq[line].port = 0;
    s_irq[line].used = false;

    return ABL_STATUS_OK;
}

/* Any EXTI vector scans the armed lines: the HAL checks and clears the pending
 * bit and calls the registered trampoline. */
static void exti_dispatch(void)
{
    for (uint32_t line = 0; line < ABL_GPIO_IRQ_LINES; line++) {
        if (s_irq[line].used) {
            (void)HAL_EXTI_IRQHandler(&s_exti[line]);
        }
    }
}

void EXTI0_IRQHandler(void)     { exti_dispatch(); }
void EXTI1_IRQHandler(void)     { exti_dispatch(); }
void EXTI2_IRQHandler(void)     { exti_dispatch(); }
void EXTI3_IRQHandler(void)     { exti_dispatch(); }
void EXTI4_IRQHandler(void)     { exti_dispatch(); }
void EXTI9_5_IRQHandler(void)   { exti_dispatch(); }
void EXTI15_10_IRQHandler(void) { exti_dispatch(); }

#else  /* !HAL_EXTI_MODULE_ENABLED */

abl_status_t abl_gpio_irq_attach(const abl_gpio_pin_t* pin,
                                 abl_gpio_irq_trigger_t trigger,
                                 abl_gpio_irq_cb_t cb, void* arg)
{
    (void)pin; (void)trigger; (void)cb; (void)arg;
    return ABL_STATUS_ERROR;   /* enable HAL_EXTI_MODULE_ENABLED in hal_conf */
}

abl_status_t abl_gpio_irq_detach(const abl_gpio_pin_t* pin)
{
    (void)pin;
    return ABL_STATUS_ERROR;
}

#endif /* HAL_EXTI_MODULE_ENABLED */
