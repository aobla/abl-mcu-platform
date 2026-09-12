#ifndef ABL_TEST_H
#define ABL_TEST_H

/*
 * Minimal host-test harness (D8 / ARCHITECTURE §16).
 *
 * Deliberately dependency-free: the platform must be testable offline, without
 * fetching a test framework. Each test binary is a single translation unit that
 * includes this header, defines test functions with ABL_TEST() and drives them
 * from ABL_TEST_MAIN_BEGIN()/ABL_TEST_MAIN_END().
 */

#include <stdio.h>
#include <stdlib.h>

static unsigned    abl_test_checks;
static unsigned    abl_test_failures;
static const char* abl_test_current = "";

#define ABL_TEST(name) static void name(void)

#define ABL_TEST_RUN(name)                          \
    do {                                            \
        abl_test_current = #name;                   \
        const unsigned before = abl_test_failures;  \
        name();                                     \
        if (abl_test_failures == before) {          \
            printf("  [ok]   %s\n", #name);         \
        } else {                                    \
            printf("  [FAIL] %s\n", #name);         \
        }                                           \
    } while (0)

#define ABL_CHECK(cond)                                                       \
    do {                                                                      \
        abl_test_checks++;                                                    \
        if (!(cond)) {                                                        \
            abl_test_failures++;                                              \
            printf("    %s:%d: CHECK failed: %s (in %s)\n",                   \
                   __FILE__, __LINE__, #cond, abl_test_current);              \
        }                                                                     \
    } while (0)

#define ABL_CHECK_EQ(actual, expected)                                        \
    do {                                                                      \
        abl_test_checks++;                                                    \
        const long long _actual   = (long long)(actual);                      \
        const long long _expected = (long long)(expected);                    \
        if (_actual != _expected) {                                           \
            abl_test_failures++;                                              \
            printf("    %s:%d: %s: expected %lld, got %lld (in %s)\n",        \
                   __FILE__, __LINE__, #actual, _expected, _actual,           \
                   abl_test_current);                                         \
        }                                                                     \
    } while (0)

#define ABL_TEST_MAIN_BEGIN()               \
    int main(void)                          \
    {                                       \
        printf("running %s\n", __FILE__);

#define ABL_TEST_MAIN_END()                                                   \
        printf("%u checks, %u failures\n", abl_test_checks, abl_test_failures);\
        return (abl_test_failures == 0U) ? 0 : 1;                             \
    }

#endif /* ABL_TEST_H */
