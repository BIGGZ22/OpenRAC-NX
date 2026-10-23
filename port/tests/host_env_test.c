/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * openrac/host_env.h from C, the way the library replacements read their switches: the raw value
 * of a variable of the table, NULL when it is unset, a value kept from the first read, and NULL
 * for a name the table does not have. */
#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200112L /* setenv and unsetenv */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openrac/host_env.h"

static int failures;

#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition);          \
            ++failures;                                                                            \
        }                                                                                          \
    } while (0)

/* Sets the process's variable `name` to `value`, or unsets it when `value` is NULL. */
static void put_variable(const char* name, const char* value) {
#if defined(_WIN32)
    /* Windows unsets a variable that is given the empty string. */
    CHECK(_putenv_s(name, value != NULL ? value : "") == 0);
#else
    if (value == NULL) {
        CHECK(unsetenv(name) == 0);
        return;
    }
    CHECK(setenv(name, value, 1) == 0);
#endif
}

/* Returns whether `value` is the string `expected`, NULL being no string. */
static int is(const char* value, const char* expected) {
    return value != NULL && strcmp(value, expected) == 0;
}

int main(void) {
    /* Set before the first read, which takes every variable of the table at once. */
    put_variable("OPENRAC_TRACE_CD", "1");
    put_variable("OPENRAC_SOUND_LOG", "0");
    put_variable("OPENRAC_TRACE_WAD", NULL);
    put_variable("OPENRAC_UI_SKIP", "hud,menu");
#if !defined(_WIN32)
    put_variable("OPENRAC_TRACE_SIF", ""); /* Windows cannot hold an empty variable */
#endif
    put_variable("OPENRAC_HOST_ENV_TEST_NOT_IN_TABLE", "1");

    /* The raw value, as getenv gave it. */
    CHECK(is(openrac_env_string("OPENRAC_TRACE_CD"), "1"));
    CHECK(is(openrac_env_string("OPENRAC_UI_SKIP"), "hud,menu"));

    /* "0" is a value like any other: a caller that tests against NULL still takes it as set. */
    CHECK(is(openrac_env_string("OPENRAC_SOUND_LOG"), "0"));

    /* Unset is NULL; set to the empty string is "", which kernel.c's trace takes as off. */
    CHECK(openrac_env_string("OPENRAC_TRACE_WAD") == NULL);
#if !defined(_WIN32)
    CHECK(is(openrac_env_string("OPENRAC_TRACE_SIF"), ""));
#endif

    /* The same variable gives the same string at each call, which lives as long as the program. */
    CHECK(openrac_env_string("OPENRAC_TRACE_CD") == openrac_env_string("OPENRAC_TRACE_CD"));

    /* Kept from the first read: a change made while the program runs is not seen. */
    put_variable("OPENRAC_TRACE_WAD", "1");
    put_variable("OPENRAC_TRACE_CD", NULL);
    CHECK(openrac_env_string("OPENRAC_TRACE_WAD") == NULL);
    CHECK(is(openrac_env_string("OPENRAC_TRACE_CD"), "1"));

    /* A name the table does not have is NULL even when set, at every call (logged once); so are
       a name without its OPENRAC_, one in the wrong case, and no name at all. */
    CHECK(openrac_env_string("OPENRAC_HOST_ENV_TEST_NOT_IN_TABLE") == NULL);
    CHECK(openrac_env_string("OPENRAC_HOST_ENV_TEST_NOT_IN_TABLE") == NULL);
    CHECK(openrac_env_string("TRACE_CD") == NULL);
    CHECK(openrac_env_string("openrac_trace_cd") == NULL);
    CHECK(openrac_env_string(NULL) == NULL);

    if (failures != 0) {
        fprintf(stderr, "%d check(s) failed\n", failures);
        return 1;
    }
    return 0;
}
