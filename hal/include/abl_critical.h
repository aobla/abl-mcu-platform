#ifndef ABL_CRITICAL_H
#define ABL_CRITICAL_H

#include <stdint.h>

/**
 * @brief Saved interrupt state (platform specific).
 *
 * Treat it as opaque: only pass it back to abl_critical_exit().
 */
typedef uint32_t abl_critical_state_t;

/**
 * @brief Enter a critical section (interrupts disabled).
 *
 * Returns the previous interrupt state, so nested critical sections work
 * correctly. Critical sections must be short and must never sleep.
 */
abl_critical_state_t abl_critical_enter(void);

/** @brief Leave a critical section, restoring the state returned by enter. */
void abl_critical_exit(abl_critical_state_t state);

#endif /* ABL_CRITICAL_H */
