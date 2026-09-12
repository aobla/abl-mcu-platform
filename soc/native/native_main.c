/**
 * @brief Native (host) trampoline (D7).
 *
 * Host tests define their own main() and therefore link the HAL/runtime without
 * the soc component; only the runnable native build (blink_native) gets this.
 */

#include "abl_app.h"

int main(void)
{
    abl_main();
    return 0;
}
