/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Copyright (c) 2026 the OpenRAC contributors */

/*
 * common.h for the host build of a game's decompiled C.
 *
 * The decompilations include "common.h" for their types and for the macros
 * that steer the retail compiler. This file stands in front of a game's own
 * one in the include path when the same sources are compiled to run in the
 * runtime: the types keep the console's sizes, the steering macros mean
 * nothing, assembly is never included, and the three quadword helpers that
 * are inline assembly for the console are plain C.
 *
 * The target is a 32-bit sandbox (wasm32): pointers are 4 bytes, as on the
 * console. `long` is 8 bytes there and 4 here, so the build defines `long`
 * as `long long` on the command line.
 */
#ifndef COMMON_H
#define COMMON_H

#define INCLUDE_ASM_H
#define INCLUDE_ASM(FOLDER, NAME)
#define INCLUDE_RODATA(FOLDER, NAME)
#define ASM_FUNC(FOLDER, NAME)
#define LINKER_REMNANT(FOLDER, NAME)

#include "names.h"

typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long s64; /* `long` is defined as `long long` by the build */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef float f32;
typedef double f64;

/* Where the retail compiler is told how to address a global. No meaning here. */
#define NOT_SDA
#define MACRO_ADDR

/* The console build gives the symbol a second label; here it is the symbol. */
#define SDATA(sym) __asm(#sym)

/* A 16-byte value with no alignment demand, for the copies below. */
typedef struct {
    unsigned char bytes[16];
} openrac_quad;

/* Copies one 16-byte quadword. The whole of `src` is read before `dst` is written. */
static __inline__ void qcopy(void *dst, void *src) {
    openrac_quad value = *(openrac_quad *)src;
    *(openrac_quad *)dst = value;
}

/* The same copy; the console build tells its compiler less about it. */
static __inline__ void qcopy_nc(void *dst, void *src) {
    openrac_quad value = *(openrac_quad *)src;
    *(openrac_quad *)dst = value;
}

/* Clears one 16-byte quadword. */
static __inline__ void qzero(void *p) {
    openrac_quad zero = {{0}};
    *(openrac_quad *)p = zero;
}

#endif /* COMMON_H */
