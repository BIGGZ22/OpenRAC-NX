/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The EE FPU's single-precision arithmetic in software, for C that has to compute as the console
 * does: functions hostgen translates this way (hostgen.json "ps2_float";
 * port/tools/hostgen/lower.py) and the port's own C (host/, hand/). The model is
 * port/common/ps2_float.h's: no infinities, NaNs or denormals (an exponent of 0 is zero, one of
 * 255 an ordinary number), every result cut toward zero to 24 bits, the adder drops the smaller
 * operand's bits more than one place below the larger one's last bit before it adds, an overflow
 * saturates to +-0x7FFFFFFF and an underflow is zero.
 *
 * The rest of the port computes in the host's floats with the thread rounding toward zero and
 * denormals as zero (openrac_ee_float_mode, ee.h), and openrac_fdiv's saturating divide
 * (guest.h). That is the EE's arithmetic for most of what the game computes, but not all of it:
 * the adder above, and a result past the largest float (+-0x7FFFFFFF here, an infinity or
 * FLT_MAX on the host). A function whose result is cut to an integer, or compared, where one
 * last place decides, computes through these functions instead.
 *
 * The values travel as floats holding the console's bit patterns: a pattern the host reads as a
 * NaN or an infinity (a divide by zero's 0x7FFFFFFF) is an ordinary number here, so code built
 * this way compares and converts floats only through these functions. */
#ifndef OPENRAC_PS2FLOAT_H
#define OPENRAC_PS2FLOAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* add.s, sub.s, mul.s, div.s. A divisor with exponent 0 gives +-0x7FFFFFFF, the operands' signs
   XORed; a dividend with exponent 0 gives a zero of that sign. */
float openrac_ps2_add(float a, float b);
float openrac_ps2_sub(float a, float b);
float openrac_ps2_mul(float a, float b);
float openrac_ps2_div(float a, float b);

/* sqrt.s: the root of |x|, cut (a zero for an exponent of 0). rsqrt.s: a over the cut root of
   |b|, cut again; b with exponent 0 gives +-0x7FFFFFFF by a's sign. */
float openrac_ps2_sqrt(float x);
float openrac_ps2_rsqrt(float a, float b);

/* c.lt.s, c.le.s, c.eq.s: sign and magnitude, every zero equal. 1 when true. gt and ge are lt
   and le with the operands swapped, for a > b and a >= b with their operands in source order. */
int openrac_ps2_lt(float a, float b);
int openrac_ps2_le(float a, float b);
int openrac_ps2_gt(float a, float b);
int openrac_ps2_ge(float a, float b);
int openrac_ps2_eq(float a, float b);

/* cvt.s.w: a signed integer as a float, cut to 24 bits. cvt.w.s: a float as a signed integer,
   toward zero, saturating by its sign out of range (an exponent of 255 too, which the host
   reads as a NaN); an exponent of 0 is 0. */
float openrac_ps2_itof(int32_t i);
int32_t openrac_ps2_ftoi(float x);

#ifdef __cplusplus
}
#endif

#endif /* OPENRAC_PS2FLOAT_H */
