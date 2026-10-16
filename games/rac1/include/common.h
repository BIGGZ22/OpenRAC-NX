#ifndef COMMON_H
#define COMMON_H

#include "include_asm.h"
#include "names.h"   /* readable names for func_/D_ symbols (docs/NAMES.md) */

/* Standard fixed-width types for PS2 Emotion Engine (GCC 2.95.3) */
typedef signed char s8;
typedef short s16;
typedef int s32;
typedef long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;
typedef float f32;
typedef double f64;

/*
 * Keeps a variable OUT of the small-data area.
 *
 * Retail addresses some globals via $gp (the MIPS small-data area, base
 * 0x00166D00 from its own .reginfo, window 0x15ED00..0x16ED00). To
 * reproduce those we must build with a nonzero -G, but that makes the
 * compiler use $gp for EVERY small global -- including the ~60 that live
 * far outside the window, whose references then can't reach
 * ("relocation truncated to fit: R_MIPS_GPREL16").
 *
 * Placement is decided per variable by its declared size, but the
 * incomplete-array trick (`extern char x[];`) changes how the variable
 * must be spelled at every use site, which is unacceptable for scalars
 * inside already-matching functions. An explicit section attribute
 * achieves the same thing while leaving use sites untouched: the
 * compiler knows the variable isn't in .sdata and falls back to lui/lo.
 *
 * Verified directly: a plain `extern unsigned char x;` compiles to
 * `lbu $v0,0($gp)` at -G8, the same declaration with this macro compiles
 * to `lui`/`lbu`. (`aligned` does NOT work -- it stays gp-relative.)
 */
#define NOT_SDA __attribute__((section(".data")))

/*
 * Loads a global with retail's one-register form:
 *     lui $2,%hi(D) / lw $2,%lo(D)($2)
 * where plain declarations give the split form this compiler prefers:
 *     lui $2,%hi(D) / lw $3,%lo(D)($2)
 *
 * The section name makes the compiler treat the symbol as small data, so
 * it emits the unsplit assembler macro `lw $2,D`. The declared size is
 * still over -G2, so the assembler does not use $gp for it and expands the
 * macro through the destination register. The register choice around the
 * load follows, because the compiler allocated one pseudo, not two.
 *
 * Loads only. A store to such a symbol becomes a two-instruction macro
 * (`lui $at` / `sw`) that the compiler still counts as one instruction, so
 * it can land in a delay slot; the assembler then warns "macro used after
 * .set nomacro". Check the make log for that warning.
 */
#define MACRO_ADDR __attribute__((section(".sdata")))

/*
 * A scalar in the file's own small data, declared with its real type:
 *
 *     extern float D_L17_00162108 SDATA(D_L17_00162108);
 *
 * Retail reaches a level file's own tuning floats and ints through $gp at
 * every access, in one instruction. The older way to get that here is
 * `extern short D_x;` read as `*(float *)&D_x`, and for most functions it
 * matches. But a read through a cast is not a scalar to the compiler: it may
 * alias any store through a pointer, so the load is ordered behind such
 * stores and its value is forgotten at each one. Where retail loads the
 * global before a store through a struct pointer, only the real type
 * reproduces it (func_L17_002EEB08, func_L17_002EDE50), with the stores
 * written as struct members.
 *
 * The declaration gives the symbol the assembler label `sym__gp`;
 * tools/check_macro_slots.py makes every such label a 1-byte .extern equated
 * to the symbol, so the assembler emits the $gp-relative form. A file that
 * also declares the same symbol `extern short` for an older function needs
 * two C names for it; keep the real name for this declaration.
 */
#define SDATA(sym) __asm__(#sym "__gp") MACRO_ADDR

/*
 * Copies one 16-byte quadword from src to dst through $2, the way retail's
 * own source did: an inline-asm copy shaped like libvu0's sceVu0CopyVector
 * (which uses $6). Each address goes into its own register and is read at
 * offset 0, which no C copy reproduces: a long long or aligned-struct copy
 * folds the offset into lq/sq. First matched on func_001ECC10 (camera.c).
 *
 * This is the one sanctioned inline asm. Candidates call it; they never
 * write asm inside a function themselves (tools/integrate.py refuses that).
 */
static __inline__ void qcopy(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2", "memory");
}

/*
 * The same copy without the "memory" clobber. Retail has both behaviours.
 * Where a value read before a copy is read again after it, the hero's
 * state-setting function (func_L02_0022BE40 and its level versions) reloads
 * it, which is qcopy(); the hero's update function keeps it in its register
 * across the copy (case 107 of func_L05_00256148 and func_L16_00227818),
 * which is this one. Dropping the clobber from qcopy() itself leaves eleven
 * state-setting functions 8 bytes short, so the two are separate. Use this
 * one only where retail keeps a value live across the copy.
 */
static __inline__ void qcopy_nc(void *dst, void *src) {
    __asm__ __volatile__("lq $2,0x0(%1)\n\tsq $2,0x0(%0)" : : "r"(dst), "r"(src) : "$2");
}

/*
 * Clears one 16-byte quadword with `sq $0`, the other store retail's own
 * source had as inline asm: this compiler's `sq` takes a register, so C
 * zeroing a 128-bit value always comes out `por $2,$0,$0` + `sq $2`, while
 * retail has `sq $0` in code that is otherwise the compiler's. No clobbers:
 * with a "memory" clobber the compiler reloads the base address around it.
 * Identified by Lombyte, the US decompilation (its include/qzero.h).
 */
static __inline__ void qzero(void *p) {
    __asm__ __volatile__("sq $0,0x0(%0)" : : "r"(p));
}

#endif /* COMMON_H */
