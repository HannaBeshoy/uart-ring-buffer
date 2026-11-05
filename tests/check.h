/* check.h - a minimal test framework: CHECK(cond) counts and reports failures. */
#ifndef CHECK_H
#define CHECK_H
#include <stdio.h>

static int checks_run = 0, checks_failed = 0;

#define CHECK(cond) do { checks_run++; if (!(cond)) { checks_failed++; \
    fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)

#define TEST(name) static void name(void)
#define RUN(name) do { int before = checks_failed; name(); \
    printf("%s  %s\n", checks_failed == before ? "PASS" : "FAIL", #name); } while (0)

#define REPORT() (printf("\n%d checks, %d failed\n", checks_run, checks_failed), checks_failed != 0)
#endif
