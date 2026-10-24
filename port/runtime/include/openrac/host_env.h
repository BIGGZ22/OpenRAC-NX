/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The port's OPENRAC_* environment variables, for C: the library replacements of
 * port/game/common/lib and the games' host C, which cannot include the C++ table they are kept in
 * (port/common/env.h; docs/CODING_CONVENTIONS.md, section 14.3). Every OPENRAC_* variable is read
 * through that table, so a variable C reads is a row of it, documented in docs/port/DEBUGGING.md
 * like every other; the C++ side of this contract is runtime/host_env.cpp.
 *
 * The variables are read once, all together, the first time any of them is asked for (from C or
 * from C++), and kept for the life of the program: they are set before the program starts, and
 * nothing in the port sets one while it runs. Any thread may call these functions. */
#ifndef OPENRAC_HOST_ENV_H
#define OPENRAC_HOST_ENV_H

#ifdef __cplusplus
extern "C" {
#endif

/* The value the variable `name` (its whole name, "OPENRAC_TRACE_CD") was set to, or NULL when it
 * is unset. A variable set to the empty string gives "", not NULL. The string lives as long as
 * the program, and the same variable gives the same pointer at each call.
 *
 * This is the raw value, as the C library's environment held it: each caller tests it as it always
 * has. A caller that tested its old read against NULL tests this one against NULL, and so still
 * takes "0" as set, whatever the table's row says turns the variable on.
 *
 * A name the table does not have (a misspelling, a variable never added to it) is a programming
 * error: it is reported once in the log, as an error naming it, and gives NULL at every call, so
 * the switch it was meant to be stays off and the run goes on. */
const char* openrac_env_string(const char* name);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_HOST_ENV_H */
