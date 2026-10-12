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

/* Quadword copies and clears, as the game's macros wrote them (lq/sq). */
static __inline__ void qcopy(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}
static __inline__ void qcopy_nc(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2");
}
static __inline__ void qzero(void *p) {
    __asm__ __volatile__("sq $0,0x0(%0)" : : "r"(p));
}

#endif /* COMMON_H */
