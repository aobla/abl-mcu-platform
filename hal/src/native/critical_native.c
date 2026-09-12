/**
 * @brief Native (host) critical sections (L1).
 *
 * The host has no interrupt masking; a nesting counter is kept so that tests can
 * assert the pairing of enter/exit without changing program behaviour.
 */

#include "abl_critical.h"

static unsigned s_nesting;

abl_critical_state_t abl_critical_enter(void)
{
    const unsigned previous = s_nesting;
    s_nesting++;
    return (abl_critical_state_t)previous;
}

void abl_critical_exit(abl_critical_state_t state)
{
    if (s_nesting > 0U) {
        s_nesting--;
    }
    (void)state;
}
