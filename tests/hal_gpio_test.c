/*
 * abl_gpio contract tests on the native (host) port.
 *
 * Exercises the parts of the contract that are platform-independent plus the
 * error paths: configuration, output level, toggle, simulated input, and the
 * unsupported-interrupt behaviour of the host port.
 */

#include "abl_test.h"

#include "abl_gpio.h"
#include "abl_native_sim.h"

static const abl_gpio_pin_t s_led  = { (void*)(uintptr_t)0, 3 };
static const abl_gpio_pin_t s_btn  = { (void*)(uintptr_t)0, 4 };
static const abl_gpio_pin_t s_free = { (void*)(uintptr_t)0, 9 };

ABL_TEST(configure_output_and_write)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = ABL_GPIO_MODE_OUTPUT;
    cfg.state = ABL_GPIO_STATE_LOW;

    ABL_CHECK_EQ(abl_gpio_configure(&s_led, &cfg), ABL_STATUS_OK);

    bool level = true;
    ABL_CHECK_EQ(abl_gpio_read(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, false);

    ABL_CHECK_EQ(abl_gpio_write(&s_led, true), ABL_STATUS_OK);
    ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, true);

    ABL_CHECK_EQ(abl_gpio_write(&s_led, false), ABL_STATUS_OK);
    ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, false);
}

ABL_TEST(toggle_flips_level)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = ABL_GPIO_MODE_OUTPUT;
    cfg.state = ABL_GPIO_STATE_LOW;
    ABL_CHECK_EQ(abl_gpio_configure(&s_led, &cfg), ABL_STATUS_OK);

    ABL_CHECK_EQ(abl_gpio_toggle(&s_led), ABL_STATUS_OK);

    bool level = false;
    ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, true);

    ABL_CHECK_EQ(abl_gpio_toggle(&s_led), ABL_STATUS_OK);
    ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, false);
}

ABL_TEST(simulated_input_is_visible)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode = ABL_GPIO_MODE_INPUT;
    cfg.pull = ABL_GPIO_PULL_UP;
    ABL_CHECK_EQ(abl_gpio_configure(&s_btn, &cfg), ABL_STATUS_OK);

    ABL_CHECK_EQ(abl_native_gpio_inject(&s_btn, false), ABL_STATUS_OK);

    bool level = true;
    ABL_CHECK_EQ(abl_gpio_read(&s_btn, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, false);
}

ABL_TEST(toggle_ignores_the_pad_level)
{
    /* Regression test for a real ESP32-C3 bug: the port used to implement
     * toggle() by reading the pad back (gpio_get_level) and inverting it. With a
     * loaded output the pad does not follow the driven value, so toggling became
     * unreliable. toggle() must depend only on the level we drove.
     *
     * Here the "pad" is externally forced low while we drive high: a correct
     * implementation still toggles to low on the next call instead of repeating
     * high. */
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = ABL_GPIO_MODE_OUTPUT;
    cfg.state = ABL_GPIO_STATE_LOW;
    ABL_CHECK_EQ(abl_gpio_configure(&s_led, &cfg), ABL_STATUS_OK);

    ABL_CHECK_EQ(abl_gpio_write(&s_led, true), ABL_STATUS_OK);

    /* Simulate a pad that is dragged low by the load. */
    ABL_CHECK_EQ(abl_native_gpio_inject(&s_led, false), ABL_STATUS_OK);

    ABL_CHECK_EQ(abl_gpio_toggle(&s_led), ABL_STATUS_OK);

    bool level = true;
    ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
    ABL_CHECK_EQ(level, false);   /* driven high -> toggle must go low */
}

ABL_TEST(toggle_after_config_is_deterministic)
{
    /* toggle() on a freshly configured pin must produce a defined sequence,
     * regardless of what the pad reported before. */
    abl_gpio_cfg_t cfg = {0};
    cfg.mode  = ABL_GPIO_MODE_OUTPUT;
    cfg.state = ABL_GPIO_STATE_LOW;
    ABL_CHECK_EQ(abl_gpio_configure(&s_led, &cfg), ABL_STATUS_OK);

    bool level = false;
    for (unsigned i = 0; i < 4U; i++) {
        ABL_CHECK_EQ(abl_gpio_toggle(&s_led), ABL_STATUS_OK);
        ABL_CHECK_EQ(abl_native_gpio_get(&s_led, &level), ABL_STATUS_OK);
        ABL_CHECK_EQ(level, (i % 2U) == 0U);
    }
}

ABL_TEST(error_paths)
{
    abl_gpio_cfg_t cfg = {0};
    cfg.mode = ABL_GPIO_MODE_OUTPUT;

    /* NULL arguments */
    ABL_CHECK_EQ(abl_gpio_configure(0, &cfg), ABL_STATUS_ERROR);
    ABL_CHECK_EQ(abl_gpio_configure(&s_led, 0), ABL_STATUS_ERROR);
    ABL_CHECK_EQ(abl_gpio_read(&s_led, 0), ABL_STATUS_ERROR);

    /* invalid mode */
    cfg.mode = (abl_gpio_mode_t)99;
    ABL_CHECK_EQ(abl_gpio_configure(&s_led, &cfg), ABL_STATUS_ERROR);

    /* writing/toggling a pin that was never configured */
    ABL_CHECK_EQ(abl_gpio_write(&s_free, true), ABL_STATUS_ERROR);
    ABL_CHECK_EQ(abl_gpio_toggle(&s_free), ABL_STATUS_ERROR);
}

ABL_TEST(interrupts_are_unsupported_on_host)
{
    /* The contract documents ABL_STATUS_ERROR where the platform has no
     * per-pin interrupts: the host port must report it instead of pretending. */
    ABL_CHECK_EQ(abl_gpio_irq_attach(&s_btn, ABL_GPIO_IRQ_RISING, 0, 0), ABL_STATUS_ERROR);
    ABL_CHECK_EQ(abl_gpio_irq_detach(&s_btn), ABL_STATUS_ERROR);
}

ABL_TEST_MAIN_BEGIN()
    ABL_TEST_RUN(configure_output_and_write);
    ABL_TEST_RUN(toggle_flips_level);
    ABL_TEST_RUN(toggle_ignores_the_pad_level);
    ABL_TEST_RUN(toggle_after_config_is_deterministic);
    ABL_TEST_RUN(simulated_input_is_visible);
    ABL_TEST_RUN(error_paths);
    ABL_TEST_RUN(interrupts_are_unsupported_on_host);
ABL_TEST_MAIN_END()
