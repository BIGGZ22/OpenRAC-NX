#ifndef COMMON_H
#define COMMON_H

/* Shared with rac1-decomp's include/common.h: the same types, markers and attributes. */
#include "include_asm.h"
#include "names.h"   /* readable names for func_ symbols */

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef signed char        s8;
typedef signed short       s16;
typedef signed int         s32;
typedef signed long long   s64;

typedef float              f32;
typedef double             f64;

/* A global the compiler must not address $gp-relative (it is not in the small-data area). */
#define NOT_SDA __attribute__((section(".data")))
/* A global in the small-data area, addressed $gp-relative. */
#define MACRO_ADDR __attribute__((section(".sdata")))
#define SDATA(sym) __asm__(#sym "__gp") MACRO_ADDR

#endif /* COMMON_H */
