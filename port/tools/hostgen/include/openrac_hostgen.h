/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * Included first in every unit hostgen reads (clangast.py). An inline
 * assembly statement that Clang cannot read (a register constraint only GCC
 * knows) is replaced by a call to this function in the prepared copy, so the
 * unit still reads; hostgen makes the function that contained it a stub.
 *
 * openrac_ti and openrac_uti are the 128-bit integers of `int __attribute__((mode(TI)))`
 * written inside a cast, where Clang drops the attribute and leaves an int; the prepared
 * copy names these instead (prep.py, name_wide_casts). */
#ifndef OPENRAC_HOSTGEN_H
#define OPENRAC_HOSTGEN_H
void openrac_hostgen_asm(void);
typedef int openrac_ti __attribute__((mode(TI)));
typedef unsigned int openrac_uti __attribute__((mode(TI)));
#endif
