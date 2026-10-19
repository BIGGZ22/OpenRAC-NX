/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * C translated from assembly (asmc.py), for the console's own compiler: an offline check of a
 * translation against retail, by rac1-decomp's tools/fcheck.py, which builds a candidate in its
 * decompilation file and runs it and retail from the same captured call states. Never part of
 * the port. Memory is the console's own, calls go to retail addresses, and the vector unit's
 * registers are the real ones: copied into the function's own state at entry and after each
 * call, and back before each call and at each return. The FPU's divide, roots and conversion
 * are its own instructions.
 *
 * hostgen writes this file's text, then ee_ops.h's, before each candidate (asmc.console_text).
 */
#ifndef OPENRAC_ASMC_CONSOLE_H
#define OPENRAC_ASMC_CONSOLE_H

typedef unsigned char openrac_u8;
typedef signed char openrac_s8;
typedef unsigned short openrac_u16;
typedef short openrac_s16;
typedef unsigned int openrac_u32;
typedef int openrac_s32;
typedef unsigned long openrac_u64; /* the EE's long is 64 bits */
typedef long openrac_s64;
typedef float openrac_f32;
typedef unsigned int gaddr;

#define OPENRAC_EE_INLINE static __inline__
#define OPENRAC_EE_PTR(a) ((void *)(openrac_u32)(a))
#define OPENRAC_EE_MEM volatile

static __inline__ float openrac_ee_fdiv(float a, float b) {
    float r;
    __asm__ __volatile__("div.s %0, %1, %2" : "=f"(r) : "f"(a), "f"(b));
    return r;
}
static __inline__ float openrac_ee_sqrt(float x) {
    float r;
    __asm__ __volatile__("sqrt.s %0, %1" : "=f"(r) : "f"(x));
    return r;
}
static __inline__ float openrac_ee_rsqrt(float a, float b) {
    float r;
    __asm__ __volatile__("rsqrt.s %0, %1, %2" : "=f"(r) : "f"(a), "f"(b));
    return r;
}
static __inline__ openrac_u32 openrac_ee_cvtws(float f) {
    float r;
    openrac_u32 u;
    __asm__ __volatile__("cvt.w.s %0, %1" : "=f"(r) : "f"(f));
    __asm__ __volatile__("mfc1 %0, %1" : "=r"(u) : "f"(r));
    return u;
}

/* Nothing to report to on the console; no string goes into the candidate's data. */
#define openrac_ee_break(function, address) ((void)(address))
#define openrac_ee_lost(function, target) ((void)(target))

#endif /* OPENRAC_ASMC_CONSOLE_H */
