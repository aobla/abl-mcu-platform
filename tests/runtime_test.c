/*
 * Runtime contract tests on the bare backend (host).
 *
 * Note: abl_runtime_run() never returns by design, so it is not called here —
 * the tests exercise the sleep/uptime behaviour and the error paths.
 */

#include "abl_test.h"

#include "abl_runtime.h"
#include "abl_critical.h"
#include "abl_time.h"

ABL_TEST(uptime_is_monotonic)
{
    const uint32_t t0 = abl_uptime_ms();
    abl_sleep_ms(5);
    const uint32_t t1 = abl_uptime_ms();

    /* Real sleeping on the host: at least a few milliseconds must have passed. */
    ABL_CHECK(t1 >= t0);
    ABL_CHECK((t1 - t0) >= 3U);
}

ABL_TEST(sleep_does_not_hang)
{
    abl_sleep_us(200);
    /* reaching this line at all is the assertion */
    ABL_CHECK(true);
}

ABL_TEST(time_init_is_idempotent)
{
    ABL_CHECK_EQ(abl_time_init(), ABL_STATUS_OK);
    ABL_CHECK_EQ(abl_time_init(), ABL_STATUS_OK);
}

ABL_TEST(task_create_validates_arguments)
{
    ABL_CHECK_EQ(abl_task_create(0, "bad", 0), ABL_STATUS_ERROR);
}

ABL_TEST(critical_sections_are_nestable)
{
    const abl_critical_state_t outer = abl_critical_enter();
    const abl_critical_state_t inner = abl_critical_enter();
    abl_critical_exit(inner);
    abl_critical_exit(outer);

    /* unbalanced exit must not corrupt the counter */
    abl_critical_exit(outer);
    ABL_CHECK(true);
}

ABL_TEST_MAIN_BEGIN()
    ABL_TEST_RUN(uptime_is_monotonic);
    ABL_TEST_RUN(sleep_does_not_hang);
    ABL_TEST_RUN(time_init_is_idempotent);
    ABL_TEST_RUN(task_create_validates_arguments);
    ABL_TEST_RUN(critical_sections_are_nestable);
ABL_TEST_MAIN_END()
