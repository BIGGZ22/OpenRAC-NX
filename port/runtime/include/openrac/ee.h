/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * What the C hostgen writes from a function's assembly needs of the port
 * (port/tools/hostgen/asmc.py): the R5900's instructions on game memory
 * (ee_ops.h), the FPU's divide and roots as the EE computes them, the vector
 * unit's registers, and what to do where the program would have trapped.
 *
 * Such a function keeps the EE's registers in locals: integer registers as
 * 64-bit values (with a second half where it uses 128-bit ones), FPU registers
 * as floats. Its stack is the game stack (guest.h): $sp starts at
 * openrac_guest_sp, and every call it makes sees openrac_guest_sp at its $sp.
 */
#ifndef OPENRAC_EE_H
#define OPENRAC_EE_H

#include <math.h>

#include "openrac/guest.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t openrac_u8;
typedef int8_t openrac_s8;
typedef uint16_t openrac_u16;
typedef int16_t openrac_s16;
typedef uint32_t openrac_u32;
typedef int32_t openrac_s32;
typedef uint64_t openrac_u64;
typedef int64_t openrac_s64;
typedef float openrac_f32;

#define OPENRAC_EE_INLINE static inline
#define OPENRAC_EE_PTR(a) G(a)

/* div.s: no infinities (guest.h). sqrt.s takes the root of the magnitude (the EE gives no NaN);
 * rsqrt.s is a over the root of |b|, saturating like the divide. */
#define openrac_ee_fdiv(a, b) openrac_fdiv((a), (b))
static inline float openrac_ee_sqrt(float x) {
    return sqrtf(fabsf(x));
}
static inline float openrac_ee_rsqrt(float a, float b) {
    return openrac_fdiv(a, sqrtf(fabsf(b)));
}
/* cvt.w.s: truncated toward zero, saturated at the ends of the range (no exception). */
static inline uint32_t openrac_ee_cvtws(float f) {
    if (f >= 2147483647.0f) {
        return 0x7FFFFFFFu;
    }
    if (f <= -2147483648.0f || f != f) {
        return 0x80000000u;
    }
    return (uint32_t)(int32_t)f;
}

#include "openrac/ee_ops.h"

/* The one vector unit 0 the program has. */
extern openrac_vu0_state openrac_vu0;
#define OPENRAC_VU0 (&openrac_vu0)
#define OPENRAC_VU0_IN()
#define OPENRAC_VU0_OUT()

/* A break or trap instruction the function reached (a divide by zero check, an assertion of
 * the game's): logged once per place, and the function goes on, as the EE's handler returns. */
void openrac_ee_break(const char* function, uint32_t address);
/* A jump through a register to a place the function's tables do not list: logged, and the
 * function returns. */
void openrac_ee_lost(const char* function, uint32_t target);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_EE_H */
