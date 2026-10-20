/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright (c) 2026 the OpenRAC contributors
 *
 * The R5900's instructions that take more than one C expression, for the C
 * hostgen writes from a function's assembly (port/tools/hostgen/asmc.py).
 *
 * Written from the instruction set's semantics, in C that the console's own
 * compiler also reads (GNU C89: no declarations after statements, no 128-bit
 * type), because the same text is built two ways: for the port, through ee.h,
 * on game memory; and for the offline check of a translation against retail
 * (rac1-decomp's tools/fcheck.py), on the console's memory. What differs is
 * given before this file is included:
 *
 *   OPENRAC_EE_PTR(a)   the host pointer for game address a
 *   OPENRAC_EE_MEM      a qualifier for every access (volatile on the console; none by default)
 *   openrac_u8 ... openrac_s64, openrac_f32   the integer and float types
 *   OPENRAC_EE_INLINE   static inline, as the compiler spells it
 *   openrac_ee_fdiv, openrac_ee_sqrt, openrac_ee_rsqrt, openrac_ee_cvtws
 *                       the FPU's divide, roots and conversion to integer
 *
 * A 128-bit register is two 64-bit halves (lo, hi). Values in registers are as
 * the EE keeps them: a 32-bit result sign-extended to 64 bits.
 */
#ifndef OPENRAC_EE_OPS_H
#define OPENRAC_EE_OPS_H

/* How memory is reached: nothing on the host (game C is built without type-based aliasing);
 * volatile where the compiler would assume it. */
#ifndef OPENRAC_EE_MEM
#define OPENRAC_EE_MEM
#endif

#define OPENRAC_EE_S32(x) ((openrac_u64)(openrac_s64)(openrac_s32)(openrac_u32)(x))
#define OPENRAC_EE_B(a) (*(volatile openrac_u8 *)OPENRAC_EE_PTR(a))

typedef union {
    openrac_f32 f;
    openrac_u32 u;
} openrac_ee_fbits;

OPENRAC_EE_INLINE openrac_f32 openrac_ee_f(openrac_u64 bits) {
    openrac_ee_fbits b;
    b.u = (openrac_u32)bits;
    return b.f;
}

OPENRAC_EE_INLINE openrac_u32 openrac_ee_bits(openrac_f32 f) {
    openrac_ee_fbits b;
    b.f = f;
    return b.u;
}

/* A double is software on the EE: its 64 bits travel in one general register (libgcc's
 * soft-float routines take and give them there), where the host's C has a double. */
typedef union {
    double d;
    openrac_u64 u;
} openrac_ee_dbits_u;

OPENRAC_EE_INLINE double openrac_ee_d(openrac_u64 bits) {
    openrac_ee_dbits_u b;
    b.u = bits;
    return b.d;
}

OPENRAC_EE_INLINE openrac_u64 openrac_ee_dbits(double d) {
    openrac_ee_dbits_u b;
    b.d = d;
    return b.u;
}

/* ---- Memory ---- */

OPENRAC_EE_INLINE openrac_u64 openrac_ee_lb(openrac_u32 a) {
    return (openrac_u64)(openrac_s64) * (OPENRAC_EE_MEM openrac_s8 *)OPENRAC_EE_PTR(a);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lbu(openrac_u32 a) {
    return *(OPENRAC_EE_MEM openrac_u8 *)OPENRAC_EE_PTR(a);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lh(openrac_u32 a) {
    return (openrac_u64)(openrac_s64) * (OPENRAC_EE_MEM openrac_s16 *)OPENRAC_EE_PTR(a & ~1u);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lhu(openrac_u32 a) {
    return *(OPENRAC_EE_MEM openrac_u16 *)OPENRAC_EE_PTR(a & ~1u);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lw(openrac_u32 a) {
    return (openrac_u64)(openrac_s64) * (OPENRAC_EE_MEM openrac_s32 *)OPENRAC_EE_PTR(a & ~3u);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lwu(openrac_u32 a) {
    return *(OPENRAC_EE_MEM openrac_u32 *)OPENRAC_EE_PTR(a & ~3u);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_ld(openrac_u32 a) {
    return *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a & ~7u);
}
OPENRAC_EE_INLINE openrac_f32 openrac_ee_lwc1(openrac_u32 a) {
    return *(OPENRAC_EE_MEM openrac_f32 *)OPENRAC_EE_PTR(a & ~3u);
}
OPENRAC_EE_INLINE void openrac_ee_sb(openrac_u32 a, openrac_u64 v) {
    *(OPENRAC_EE_MEM openrac_u8 *)OPENRAC_EE_PTR(a) = (openrac_u8)v;
}
OPENRAC_EE_INLINE void openrac_ee_sh(openrac_u32 a, openrac_u64 v) {
    *(OPENRAC_EE_MEM openrac_u16 *)OPENRAC_EE_PTR(a & ~1u) = (openrac_u16)v;
}
OPENRAC_EE_INLINE void openrac_ee_sw(openrac_u32 a, openrac_u64 v) {
    *(OPENRAC_EE_MEM openrac_u32 *)OPENRAC_EE_PTR(a & ~3u) = (openrac_u32)v;
}
OPENRAC_EE_INLINE void openrac_ee_sd(openrac_u32 a, openrac_u64 v) {
    *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a & ~7u) = v;
}
OPENRAC_EE_INLINE void openrac_ee_swc1(openrac_u32 a, openrac_f32 v) {
    *(OPENRAC_EE_MEM openrac_f32 *)OPENRAC_EE_PTR(a & ~3u) = v;
}
/* lq and sq ignore the low four bits of the address. */
OPENRAC_EE_INLINE void openrac_ee_lq(openrac_u32 a, openrac_u64 *lo, openrac_u64 *hi) {
    a &= ~15u;
    *lo = *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a);
    *hi = *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a + 8u);
}
OPENRAC_EE_INLINE void openrac_ee_sq(openrac_u32 a, openrac_u64 lo, openrac_u64 hi) {
    a &= ~15u;
    *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a) = lo;
    *(OPENRAC_EE_MEM openrac_u64 *)OPENRAC_EE_PTR(a + 8u) = hi;
}

/* The unaligned halves (little-endian): lwl fills the register's high bytes from the aligned
 * word's bytes up to the address, lwr its low bytes from the address up; swl and swr store the
 * same bytes back. Byte by byte, as the definition reads. */
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lwl(openrac_u32 a, openrac_u64 old) {
    openrac_u32 k = a & 3u, base = a & ~3u, v = (openrac_u32)old, j;
    for (j = 0; j <= k; j++) {
        openrac_u32 at = 8u * (3u - k + j);
        v = (v & ~(0xFFu << at)) | ((openrac_u32)OPENRAC_EE_B(base + j) << at);
    }
    return OPENRAC_EE_S32(v);
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_lwr(openrac_u32 a, openrac_u64 old) {
    openrac_u32 k = a & 3u, v = (openrac_u32)old, j;
    for (j = 0; j <= 3u - k; j++) {
        v = (v & ~(0xFFu << (8u * j))) | ((openrac_u32)OPENRAC_EE_B(a + j) << (8u * j));
    }
    /* A whole word is sign-extended; part of one leaves the upper half as it was. */
    return k == 0 ? OPENRAC_EE_S32(v) : ((old & 0xFFFFFFFF00000000ull) | v);
}
OPENRAC_EE_INLINE void openrac_ee_swl(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 3u, base = a & ~3u, j;
    for (j = 0; j <= k; j++) {
        OPENRAC_EE_B(base + j) = (openrac_u8)((openrac_u32)v >> (8u * (3u - k + j)));
    }
}
OPENRAC_EE_INLINE void openrac_ee_swr(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 3u, j;
    for (j = 0; j <= 3u - k; j++) {
        OPENRAC_EE_B(a + j) = (openrac_u8)((openrac_u32)v >> (8u * j));
    }
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_ldl(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 7u, base = a & ~7u, j;
    for (j = 0; j <= k; j++) {
        openrac_u32 at = 8u * (7u - k + j);
        v = (v & ~((openrac_u64)0xFF << at)) | ((openrac_u64)OPENRAC_EE_B(base + j) << at);
    }
    return v;
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_ldr(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 7u, j;
    for (j = 0; j <= 7u - k; j++) {
        v = (v & ~((openrac_u64)0xFF << (8u * j))) | ((openrac_u64)OPENRAC_EE_B(a + j) << (8u * j));
    }
    return v;
}
OPENRAC_EE_INLINE void openrac_ee_sdl(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 7u, base = a & ~7u, j;
    for (j = 0; j <= k; j++) {
        OPENRAC_EE_B(base + j) = (openrac_u8)(v >> (8u * (7u - k + j)));
    }
}
OPENRAC_EE_INLINE void openrac_ee_sdr(openrac_u32 a, openrac_u64 v) {
    openrac_u32 k = a & 7u, j;
    for (j = 0; j <= 7u - k; j++) {
        OPENRAC_EE_B(a + j) = (openrac_u8)(v >> (8u * j));
    }
}

/* ---- Multiply and divide ---- */

/* HI and LO each get one 32-bit half of the product, sign-extended. */
OPENRAC_EE_INLINE openrac_u64 openrac_ee_mult(openrac_u64 s, openrac_u64 t, openrac_u64 *lo, openrac_u64 *hi) {
    openrac_s64 p = (openrac_s64)(openrac_s32)s * (openrac_s64)(openrac_s32)t;
    *lo = OPENRAC_EE_S32((openrac_u64)p);
    *hi = OPENRAC_EE_S32((openrac_u64)p >> 32);
    return *lo;
}
OPENRAC_EE_INLINE openrac_u64 openrac_ee_multu(openrac_u64 s, openrac_u64 t, openrac_u64 *lo, openrac_u64 *hi) {
    openrac_u64 p = (openrac_u64)(openrac_u32)s * (openrac_u64)(openrac_u32)t;
    *lo = OPENRAC_EE_S32(p);
    *hi = OPENRAC_EE_S32(p >> 32);
    return *lo;
}
/* madd: HI:LO plus the product, each half sign-extended again. */
OPENRAC_EE_INLINE openrac_u64 openrac_ee_madd(openrac_u64 s, openrac_u64 t, openrac_u64 *lo, openrac_u64 *hi, int is_unsigned) {
    openrac_u64 acc = ((openrac_u64)(openrac_u32)*hi << 32) | (openrac_u32)*lo;
    if (is_unsigned) {
        acc += (openrac_u64)(openrac_u32)s * (openrac_u64)(openrac_u32)t;
    } else {
        acc += (openrac_u64)((openrac_s64)(openrac_s32)s * (openrac_s64)(openrac_s32)t);
    }
    *lo = OPENRAC_EE_S32(acc);
    *hi = OPENRAC_EE_S32(acc >> 32);
    return *lo;
}
/* A divisor of zero leaves LO -1 (or 1 for a negative dividend) and HI the dividend; the most
 * negative number over -1 is itself, remainder 0. */
OPENRAC_EE_INLINE void openrac_ee_div(openrac_u64 s, openrac_u64 t, openrac_u64 *lo, openrac_u64 *hi) {
    openrac_s32 a = (openrac_s32)s, b = (openrac_s32)t;
    if (b == 0) {
        *lo = OPENRAC_EE_S32(a < 0 ? 1 : -1);
        *hi = OPENRAC_EE_S32(a);
    } else if (a == (openrac_s32)0x80000000 && b == -1) {
        *lo = OPENRAC_EE_S32(a);
        *hi = 0;
    } else {
        *lo = OPENRAC_EE_S32(a / b);
        *hi = OPENRAC_EE_S32(a % b);
    }
}
OPENRAC_EE_INLINE void openrac_ee_divu(openrac_u64 s, openrac_u64 t, openrac_u64 *lo, openrac_u64 *hi) {
    openrac_u32 a = (openrac_u32)s, b = (openrac_u32)t;
    if (b == 0) {
        *lo = OPENRAC_EE_S32(0xFFFFFFFFu);
        *hi = OPENRAC_EE_S32(a);
    } else {
        *lo = OPENRAC_EE_S32(a / b);
        *hi = OPENRAC_EE_S32(a % b);
    }
}

/* ---- FPU ---- */

OPENRAC_EE_INLINE openrac_f32 openrac_ee_max(openrac_f32 a, openrac_f32 b) {
    return a > b ? a : b;
}
OPENRAC_EE_INLINE openrac_f32 openrac_ee_min(openrac_f32 a, openrac_f32 b) {
    return a < b ? a : b;
}
/* abs.s and neg.s work on the sign bit. */
OPENRAC_EE_INLINE openrac_f32 openrac_ee_fabs(openrac_f32 a) {
    return openrac_ee_f(openrac_ee_bits(a) & 0x7FFFFFFFu);
}
OPENRAC_EE_INLINE openrac_f32 openrac_ee_fneg(openrac_f32 a) {
    return openrac_ee_f(openrac_ee_bits(a) ^ 0x80000000u);
}

/* ---- Multimedia (MMI): 128-bit registers as lanes ---- */

typedef struct {
    openrac_u64 lo, hi;
} openrac_ee_q;

OPENRAC_EE_INLINE openrac_u32 openrac_ee_w(openrac_ee_q q, int i) {
    return (openrac_u32)((i < 2 ? q.lo : q.hi) >> (32 * (i & 1)));
}
OPENRAC_EE_INLINE void openrac_ee_setw(openrac_ee_q *q, int i, openrac_u32 v) {
    openrac_u64 *half = i < 2 ? &q->lo : &q->hi;
    int at = 32 * (i & 1);
    *half = (*half & ~((openrac_u64)0xFFFFFFFFu << at)) | ((openrac_u64)v << at);
}
OPENRAC_EE_INLINE openrac_u32 openrac_ee_h(openrac_ee_q q, int i) {
    return (openrac_u32)((i < 4 ? q.lo : q.hi) >> (16 * (i & 3))) & 0xFFFFu;
}
OPENRAC_EE_INLINE void openrac_ee_seth(openrac_ee_q *q, int i, openrac_u32 v) {
    openrac_u64 *half = i < 4 ? &q->lo : &q->hi;
    int at = 16 * (i & 3);
    *half = (*half & ~((openrac_u64)0xFFFFu << at)) | ((openrac_u64)(v & 0xFFFFu) << at);
}
OPENRAC_EE_INLINE openrac_u32 openrac_ee_b(openrac_ee_q q, int i) {
    return (openrac_u32)((i < 8 ? q.lo : q.hi) >> (8 * (i & 7))) & 0xFFu;
}
OPENRAC_EE_INLINE void openrac_ee_setb(openrac_ee_q *q, int i, openrac_u32 v) {
    openrac_u64 *half = i < 8 ? &q->lo : &q->hi;
    int at = 8 * (i & 7);
    *half = (*half & ~((openrac_u64)0xFFu << at)) | ((openrac_u64)(v & 0xFFu) << at);
}
OPENRAC_EE_INLINE openrac_ee_q openrac_ee_qof(openrac_u64 lo, openrac_u64 hi) {
    openrac_ee_q q;
    q.lo = lo;
    q.hi = hi;
    return q;
}

/* One MMI operation: d from s and t (rs and rt), sa the shift amount. Operations by number,
 * openrac_ee_mmi_* below; asmc.py writes the number. */
enum {
    OPENRAC_MMI_PADDW, OPENRAC_MMI_PSUBW, OPENRAC_MMI_PCGTW, OPENRAC_MMI_PMAXW, OPENRAC_MMI_PMINW,
    OPENRAC_MMI_PCEQW, OPENRAC_MMI_PADDH, OPENRAC_MMI_PSUBH, OPENRAC_MMI_PCGTH, OPENRAC_MMI_PMAXH,
    OPENRAC_MMI_PMINH, OPENRAC_MMI_PCEQH, OPENRAC_MMI_PADDB, OPENRAC_MMI_PSUBB, OPENRAC_MMI_PCGTB,
    OPENRAC_MMI_PCEQB, OPENRAC_MMI_PEXTLW, OPENRAC_MMI_PEXTUW, OPENRAC_MMI_PEXTLH, OPENRAC_MMI_PEXTUH,
    OPENRAC_MMI_PEXTLB, OPENRAC_MMI_PEXTUB, OPENRAC_MMI_PPACW, OPENRAC_MMI_PPACH, OPENRAC_MMI_PPACB,
    OPENRAC_MMI_PCPYLD, OPENRAC_MMI_PCPYUD, OPENRAC_MMI_PCPYH, OPENRAC_MMI_PAND, OPENRAC_MMI_POR,
    OPENRAC_MMI_PXOR, OPENRAC_MMI_PNOR, OPENRAC_MMI_PSLLW, OPENRAC_MMI_PSRLW, OPENRAC_MMI_PSRAW,
    OPENRAC_MMI_PSLLH, OPENRAC_MMI_PSRLH, OPENRAC_MMI_PSRAH, OPENRAC_MMI_PEXEW, OPENRAC_MMI_PEXCW,
    OPENRAC_MMI_PROT3W, OPENRAC_MMI_PEXEH, OPENRAC_MMI_PEXCH, OPENRAC_MMI_PREVH, OPENRAC_MMI_PABSW,
    OPENRAC_MMI_PABSH, OPENRAC_MMI_PADDUW, OPENRAC_MMI_PSUBUW, OPENRAC_MMI_PADDUH, OPENRAC_MMI_PSUBUH,
    OPENRAC_MMI_PADDUB, OPENRAC_MMI_PSUBUB, OPENRAC_MMI_PADDSW, OPENRAC_MMI_PSUBSW, OPENRAC_MMI_PADDSH,
    OPENRAC_MMI_PSUBSH, OPENRAC_MMI_PADDSB, OPENRAC_MMI_PSUBSB, OPENRAC_MMI_PLZCW, OPENRAC_MMI_PEXT5,
    OPENRAC_MMI_PPAC5, OPENRAC_MMI_PINTH, OPENRAC_MMI_PINTEH
};

OPENRAC_EE_INLINE openrac_s64 openrac_ee_clamp(openrac_s64 v, openrac_s64 lo, openrac_s64 hi) {
    return v < lo ? lo : v > hi ? hi : v;
}

OPENRAC_EE_INLINE openrac_ee_q openrac_ee_mmi(int op, openrac_ee_q s, openrac_ee_q t, int sa) {
    openrac_ee_q d;
    int i;
    d.lo = 0;
    d.hi = 0;
    switch (op) {
    case OPENRAC_MMI_PADDW: case OPENRAC_MMI_PSUBW: case OPENRAC_MMI_PCGTW: case OPENRAC_MMI_PMAXW:
    case OPENRAC_MMI_PMINW: case OPENRAC_MMI_PCEQW: case OPENRAC_MMI_PADDUW: case OPENRAC_MMI_PSUBUW:
    case OPENRAC_MMI_PADDSW: case OPENRAC_MMI_PSUBSW: case OPENRAC_MMI_PABSW:
        for (i = 0; i < 4; i++) {
            openrac_s32 a = (openrac_s32)openrac_ee_w(s, i), b = (openrac_s32)openrac_ee_w(t, i);
            openrac_u32 ua = (openrac_u32)a, ub = (openrac_u32)b, r = 0;
            switch (op) {
            case OPENRAC_MMI_PADDW: r = ua + ub; break;
            case OPENRAC_MMI_PSUBW: r = ua - ub; break;
            case OPENRAC_MMI_PCGTW: r = a > b ? 0xFFFFFFFFu : 0; break;
            case OPENRAC_MMI_PCEQW: r = a == b ? 0xFFFFFFFFu : 0; break;
            case OPENRAC_MMI_PMAXW: r = (openrac_u32)(a > b ? a : b); break;
            case OPENRAC_MMI_PMINW: r = (openrac_u32)(a < b ? a : b); break;
            case OPENRAC_MMI_PADDUW: r = (openrac_u32)openrac_ee_clamp((openrac_s64)ua + ub, 0, 0xFFFFFFFFll); break;
            case OPENRAC_MMI_PSUBUW: r = (openrac_u32)openrac_ee_clamp((openrac_s64)ua - ub, 0, 0xFFFFFFFFll); break;
            case OPENRAC_MMI_PADDSW: r = (openrac_u32)openrac_ee_clamp((openrac_s64)a + b, -0x80000000ll, 0x7FFFFFFFll); break;
            case OPENRAC_MMI_PSUBSW: r = (openrac_u32)openrac_ee_clamp((openrac_s64)a - b, -0x80000000ll, 0x7FFFFFFFll); break;
            case OPENRAC_MMI_PABSW: r = b == (openrac_s32)0x80000000 ? 0x7FFFFFFFu : (openrac_u32)(b < 0 ? -b : b); break;
            }
            openrac_ee_setw(&d, i, r);
        }
        break;
    case OPENRAC_MMI_PADDH: case OPENRAC_MMI_PSUBH: case OPENRAC_MMI_PCGTH: case OPENRAC_MMI_PMAXH:
    case OPENRAC_MMI_PMINH: case OPENRAC_MMI_PCEQH: case OPENRAC_MMI_PADDUH: case OPENRAC_MMI_PSUBUH:
    case OPENRAC_MMI_PADDSH: case OPENRAC_MMI_PSUBSH: case OPENRAC_MMI_PABSH:
        for (i = 0; i < 8; i++) {
            openrac_s32 a = (openrac_s16)openrac_ee_h(s, i), b = (openrac_s16)openrac_ee_h(t, i);
            openrac_s32 ua = (openrac_s32)openrac_ee_h(s, i), ub = (openrac_s32)openrac_ee_h(t, i), r = 0;
            switch (op) {
            case OPENRAC_MMI_PADDH: r = a + b; break;
            case OPENRAC_MMI_PSUBH: r = a - b; break;
            case OPENRAC_MMI_PCGTH: r = a > b ? 0xFFFF : 0; break;
            case OPENRAC_MMI_PCEQH: r = a == b ? 0xFFFF : 0; break;
            case OPENRAC_MMI_PMAXH: r = a > b ? a : b; break;
            case OPENRAC_MMI_PMINH: r = a < b ? a : b; break;
            case OPENRAC_MMI_PADDUH: r = (openrac_s32)openrac_ee_clamp(ua + ub, 0, 0xFFFF); break;
            case OPENRAC_MMI_PSUBUH: r = (openrac_s32)openrac_ee_clamp(ua - ub, 0, 0xFFFF); break;
            case OPENRAC_MMI_PADDSH: r = (openrac_s32)openrac_ee_clamp(a + b, -0x8000, 0x7FFF); break;
            case OPENRAC_MMI_PSUBSH: r = (openrac_s32)openrac_ee_clamp(a - b, -0x8000, 0x7FFF); break;
            case OPENRAC_MMI_PABSH: r = b == -0x8000 ? 0x7FFF : (b < 0 ? -b : b); break;
            }
            openrac_ee_seth(&d, i, (openrac_u32)r);
        }
        break;
    case OPENRAC_MMI_PADDB: case OPENRAC_MMI_PSUBB: case OPENRAC_MMI_PCGTB: case OPENRAC_MMI_PCEQB:
    case OPENRAC_MMI_PADDUB: case OPENRAC_MMI_PSUBUB: case OPENRAC_MMI_PADDSB: case OPENRAC_MMI_PSUBSB:
        for (i = 0; i < 16; i++) {
            openrac_s32 a = (openrac_s8)openrac_ee_b(s, i), b = (openrac_s8)openrac_ee_b(t, i);
            openrac_s32 ua = (openrac_s32)openrac_ee_b(s, i), ub = (openrac_s32)openrac_ee_b(t, i), r = 0;
            switch (op) {
            case OPENRAC_MMI_PADDB: r = a + b; break;
            case OPENRAC_MMI_PSUBB: r = a - b; break;
            case OPENRAC_MMI_PCGTB: r = a > b ? 0xFF : 0; break;
            case OPENRAC_MMI_PCEQB: r = a == b ? 0xFF : 0; break;
            case OPENRAC_MMI_PADDUB: r = (openrac_s32)openrac_ee_clamp(ua + ub, 0, 0xFF); break;
            case OPENRAC_MMI_PSUBUB: r = (openrac_s32)openrac_ee_clamp(ua - ub, 0, 0xFF); break;
            case OPENRAC_MMI_PADDSB: r = (openrac_s32)openrac_ee_clamp(a + b, -0x80, 0x7F); break;
            case OPENRAC_MMI_PSUBSB: r = (openrac_s32)openrac_ee_clamp(a - b, -0x80, 0x7F); break;
            }
            openrac_ee_setb(&d, i, (openrac_u32)r);
        }
        break;
    case OPENRAC_MMI_PEXTLW:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 0)); openrac_ee_setw(&d, 1, openrac_ee_w(s, 0));
        openrac_ee_setw(&d, 2, openrac_ee_w(t, 1)); openrac_ee_setw(&d, 3, openrac_ee_w(s, 1));
        break;
    case OPENRAC_MMI_PEXTUW:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 2)); openrac_ee_setw(&d, 1, openrac_ee_w(s, 2));
        openrac_ee_setw(&d, 2, openrac_ee_w(t, 3)); openrac_ee_setw(&d, 3, openrac_ee_w(s, 3));
        break;
    case OPENRAC_MMI_PEXTLH: case OPENRAC_MMI_PEXTUH:
        for (i = 0; i < 4; i++) {
            int from = op == OPENRAC_MMI_PEXTLH ? i : i + 4;
            openrac_ee_seth(&d, 2 * i, openrac_ee_h(t, from));
            openrac_ee_seth(&d, 2 * i + 1, openrac_ee_h(s, from));
        }
        break;
    case OPENRAC_MMI_PEXTLB: case OPENRAC_MMI_PEXTUB:
        for (i = 0; i < 8; i++) {
            int from = op == OPENRAC_MMI_PEXTLB ? i : i + 8;
            openrac_ee_setb(&d, 2 * i, openrac_ee_b(t, from));
            openrac_ee_setb(&d, 2 * i + 1, openrac_ee_b(s, from));
        }
        break;
    case OPENRAC_MMI_PPACW:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 0)); openrac_ee_setw(&d, 1, openrac_ee_w(t, 2));
        openrac_ee_setw(&d, 2, openrac_ee_w(s, 0)); openrac_ee_setw(&d, 3, openrac_ee_w(s, 2));
        break;
    case OPENRAC_MMI_PPACH:
        for (i = 0; i < 4; i++) {
            openrac_ee_seth(&d, i, openrac_ee_h(t, 2 * i));
            openrac_ee_seth(&d, i + 4, openrac_ee_h(s, 2 * i));
        }
        break;
    case OPENRAC_MMI_PPACB:
        for (i = 0; i < 8; i++) {
            openrac_ee_setb(&d, i, openrac_ee_b(t, 2 * i));
            openrac_ee_setb(&d, i + 8, openrac_ee_b(s, 2 * i));
        }
        break;
    case OPENRAC_MMI_PINTH:
        for (i = 0; i < 4; i++) {
            openrac_ee_seth(&d, 2 * i, openrac_ee_h(t, i));
            openrac_ee_seth(&d, 2 * i + 1, openrac_ee_h(s, i + 4));
        }
        break;
    case OPENRAC_MMI_PINTEH:
        for (i = 0; i < 4; i++) {
            openrac_ee_seth(&d, 2 * i, openrac_ee_h(t, 2 * i));
            openrac_ee_seth(&d, 2 * i + 1, openrac_ee_h(s, 2 * i));
        }
        break;
    case OPENRAC_MMI_PCPYLD: d.lo = t.lo; d.hi = s.lo; break;
    case OPENRAC_MMI_PCPYUD: d.lo = s.hi; d.hi = t.hi; break;
    case OPENRAC_MMI_PCPYH:
        for (i = 0; i < 4; i++) {
            openrac_ee_seth(&d, i, openrac_ee_h(t, 0));
            openrac_ee_seth(&d, i + 4, openrac_ee_h(t, 4));
        }
        break;
    case OPENRAC_MMI_PAND: d.lo = s.lo & t.lo; d.hi = s.hi & t.hi; break;
    case OPENRAC_MMI_POR: d.lo = s.lo | t.lo; d.hi = s.hi | t.hi; break;
    case OPENRAC_MMI_PXOR: d.lo = s.lo ^ t.lo; d.hi = s.hi ^ t.hi; break;
    case OPENRAC_MMI_PNOR: d.lo = ~(s.lo | t.lo); d.hi = ~(s.hi | t.hi); break;
    case OPENRAC_MMI_PSLLW: case OPENRAC_MMI_PSRLW: case OPENRAC_MMI_PSRAW:
        for (i = 0; i < 4; i++) {
            openrac_u32 v = openrac_ee_w(t, i);
            int n = sa & 31;
            openrac_ee_setw(&d, i, op == OPENRAC_MMI_PSLLW ? v << n
                                   : op == OPENRAC_MMI_PSRLW ? v >> n : (openrac_u32)((openrac_s32)v >> n));
        }
        break;
    case OPENRAC_MMI_PSLLH: case OPENRAC_MMI_PSRLH: case OPENRAC_MMI_PSRAH:
        for (i = 0; i < 8; i++) {
            openrac_u32 v = openrac_ee_h(t, i);
            int n = sa & 15;
            openrac_ee_seth(&d, i, op == OPENRAC_MMI_PSLLH ? v << n
                                   : op == OPENRAC_MMI_PSRLH ? v >> n : (openrac_u32)((openrac_s16)v >> n));
        }
        break;
    case OPENRAC_MMI_PEXEW:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 2)); openrac_ee_setw(&d, 1, openrac_ee_w(t, 1));
        openrac_ee_setw(&d, 2, openrac_ee_w(t, 0)); openrac_ee_setw(&d, 3, openrac_ee_w(t, 3));
        break;
    case OPENRAC_MMI_PEXCW:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 0)); openrac_ee_setw(&d, 1, openrac_ee_w(t, 2));
        openrac_ee_setw(&d, 2, openrac_ee_w(t, 1)); openrac_ee_setw(&d, 3, openrac_ee_w(t, 3));
        break;
    case OPENRAC_MMI_PROT3W:
        openrac_ee_setw(&d, 0, openrac_ee_w(t, 1)); openrac_ee_setw(&d, 1, openrac_ee_w(t, 2));
        openrac_ee_setw(&d, 2, openrac_ee_w(t, 0)); openrac_ee_setw(&d, 3, openrac_ee_w(t, 3));
        break;
    case OPENRAC_MMI_PEXEH: case OPENRAC_MMI_PEXCH: case OPENRAC_MMI_PREVH:
        /* In each doubleword: pexeh swaps halfwords 0 and 2, pexch 1 and 2, prevh reverses them. */
        for (i = 0; i < 8; i++) {
            int k = i & 3, from;
            if (op == OPENRAC_MMI_PEXEH) {
                from = k == 0 ? 2 : k == 2 ? 0 : k;
            } else if (op == OPENRAC_MMI_PEXCH) {
                from = k == 1 ? 2 : k == 2 ? 1 : k;
            } else {
                from = 3 - k;
            }
            openrac_ee_seth(&d, i, openrac_ee_h(t, (i & 4) + from));
        }
        break;
    case OPENRAC_MMI_PLZCW:
        /* For the two low words of rs: how many bits after the first equal it, minus one. */
        for (i = 0; i < 2; i++) {
            openrac_u32 v = openrac_ee_w(s, i), sign = v >> 31;
            int n = 0;
            while (n < 31 && ((v >> (30 - n)) & 1u) == sign) {
                n++;
            }
            openrac_ee_setw(&d, i, (openrac_u32)n);
        }
        d.hi = 0;
        break;
    case OPENRAC_MMI_PEXT5:
        for (i = 0; i < 4; i++) {
            openrac_u32 v = openrac_ee_w(t, i);
            openrac_ee_setw(&d, i, ((v & 0x1Fu) << 3) | (((v >> 5) & 0x1Fu) << 11) | (((v >> 10) & 0x1Fu) << 19)
                                       | (((v >> 15) & 1u) << 31));
        }
        break;
    case OPENRAC_MMI_PPAC5:
        for (i = 0; i < 4; i++) {
            openrac_u32 v = openrac_ee_w(t, i);
            openrac_ee_setw(&d, i, ((v >> 3) & 0x1Fu) | (((v >> 11) & 0x1Fu) << 5) | (((v >> 19) & 0x1Fu) << 10)
                                       | ((v >> 31) << 15));
        }
        break;
    }
    return d;
}

/* qfsrv: the 256 bits rs:rt shifted right by SA bytes (mtsab / mtsah set SA, in bytes here),
 * the low 128 kept. */
OPENRAC_EE_INLINE openrac_ee_q openrac_ee_qfsrv(openrac_ee_q s, openrac_ee_q t, openrac_u32 sa_bytes) {
    openrac_ee_q d;
    int i;
    for (i = 0; i < 16; i++) {
        openrac_u32 k = (openrac_u32)i + (sa_bytes & 15u);
        openrac_ee_setb(&d, i, k < 16 ? openrac_ee_b(t, (int)k) : openrac_ee_b(s, (int)(k - 16)));
    }
    return d;
}

/* ---- VU0 in macro mode ----
 *
 * The vector unit's registers live as long as the program: hand-written routines leave results
 * there for the next one. vf0 is (0, 0, 0, 1) and is never written. */
typedef struct openrac_vu0_state {
    openrac_f32 vf[32][4];
    openrac_f32 acc[4];
    openrac_u32 vi[16];
    openrac_f32 q, i, r, p;
    openrac_f32 scratch[4]; /* room for moving a register in the console's form (asmc.py), 16-aligned */
} openrac_vu0_state;

/* Float to fixed point with `frac` fraction bits, as vftoi: truncated, saturated. A float with
 * exponent 255 is a large number to the VU (a NaN to the host) and saturates by its sign: the
 * texture cache of the engine's later games reads 0x80000000 as "very close". */
OPENRAC_EE_INLINE openrac_u32 openrac_ee_ftoi(openrac_f32 f, int frac) {
    openrac_f32 v = f * (openrac_f32)(1 << frac);
    if ((openrac_ee_bits(f) & 0x7F800000u) == 0x7F800000u) {
        return (openrac_ee_bits(f) >> 31) != 0 ? 0x80000000u : 0x7FFFFFFFu;
    }
    if (v >= openrac_ee_f(0x4F000000u)) { /* 2^31 */
        return 0x7FFFFFFFu;
    }
    if (v <= openrac_ee_f(0xCF000000u)) {
        return 0x80000000u;
    }
    return (openrac_u32)(openrac_s32)v;
}
OPENRAC_EE_INLINE openrac_f32 openrac_ee_itof(openrac_u32 bits, int frac) {
    return (openrac_f32)(openrac_s32)bits / (openrac_f32)(1 << frac);
}

#endif /* OPENRAC_EE_OPS_H */
