#ifndef ABL_NATIVE_SIM_H
#define ABL_NATIVE_SIM_H

#include "abl_gpio.h"

/**
 * @brief Native simulation hooks — FOR HOST TESTS ONLY.
 *
 * The native port keeps a small in-memory pin registry so that host tests can
 * drive inputs and observe outputs without hardware. Application and driver
 * code must never include this header (R1): it exists purely so that tests can
 * simulate the outside world.
 */

/** Force the level seen by abl_gpio_read() on a pin (simulated input). */
abl_status_t abl_native_gpio_inject(const abl_gpio_pin_t* pin, bool level);

/** Read back the last level written by abl_gpio_write()/toggle() (simulated output). */
abl_status_t abl_native_gpio_get(const abl_gpio_pin_t* pin, bool* level);

/** Number of pins currently registered by the simulator (test bookkeeping). */
unsigned abl_native_gpio_count(void);

#endif /* ABL_NATIVE_SIM_H */
