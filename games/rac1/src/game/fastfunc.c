#include "common.h"
#include "callees.h"
#include "records.h"
#include "structs.h"

/*
 * fastfunc.cpp in the original source; text 0x1F9810-0x1FAA28.
 * Name and boundary from the NTSC split of this game, mapped to PAL by matching function
 * sizes -- see docs/DECOMP_PROGRESS.md. Compiled as C for now.
 */

/* Declarations in scope here before the split. */
extern char D_0013E650[];
extern int D_0015F694;
extern void func_001F9A98(void *, void *, int);
extern char D_00189310[];
extern char D_001899D0[];
extern void *D_001871C0 NOT_SDA;
extern DispatchRec D_001E8F80[];
extern int D_0018A3B0[];
extern void func_001F99B0();
extern void func_001F2BC8(void);
extern int D_0018C434 NOT_SDA;
extern char D_001940C0[];
extern long D_00151888[3];
extern int D_0015F6FC;
extern short D_0015F534;
extern void func_001FB530(void);
extern void func_001F3D78(void);
extern int D_0015F564;
extern int D_0018DD40[];
extern int D_0018DC40[];
extern short D_0015F59C;
extern int func_001F65B0(unsigned char *arg0, int arg1, void *arg2);
extern unsigned char D_001DF3D0[];
extern unsigned char D_001DF770[];
extern unsigned char D_001DFB10[];
extern void func_001F6668(void *, void *, void *, void *, void *, int,
                          unsigned char *);
extern int func_001F6600(unsigned char *, int);
extern int func_001F6620(unsigned char *, int);
extern int func_001F4868(int);
extern void func_001FB498(void);
extern void func_001F3008(void);
extern void func_001F3140(void);
extern int D_0018E840[];
extern void func_001F7B40(void);

LINKER_REMNANT("asm/remnants/text", func_001F9810);

ASM_FUNC("asm/handwritten/text", func_001F9840);

ASM_FUNC("asm/handwritten/text", func_001F9850);

/*
 * Hand-written: return *(float *)&D_0015EE64 * x;  (gp 0x0015EE64)
 * as `lwc1 $f0,off($28) / nop / jr $31 / mul.s`. Nothing in the
 * toolchain writes that nop. Probed: neither GNU ee-as nor either
 * ps2eeas version puts a nop after lwc1, and gcc's own load-delay
 * "nops" are only `#nop` comments. The rest of the evidence: five
 * identical scale helpers sit here, three of them unreferenced but
 * intact, which the linker's dead-stripping does not leave of compiled
 * code; and fastfunc.o is hand-written noreorder code throughout
 * (tools/ps2eeas_nops.py exempts it). Our C comes out 12 bytes, retail
 * is 16. An older note blamed an assembler load delay.
 */
ASM_FUNC("asm/handwritten/text", func_001F9878);

ASM_FUNC("asm/handwritten/text", func_001F9888);

/*
 * Hand-written: return *(float *)&D_0015EE68 * x;  (gp 0x0015EE68)
 * as `lwc1 $f0,off($28) / nop / jr $31 / mul.s`. Nothing in the
 * toolchain writes that nop. Probed: neither GNU ee-as nor either
 * ps2eeas version puts a nop after lwc1, and gcc's own load-delay
 * "nops" are only `#nop` comments. The rest of the evidence: five
 * identical scale helpers sit here, three of them unreferenced but
 * intact, which the linker's dead-stripping does not leave of compiled
 * code; and fastfunc.o is hand-written noreorder code throughout
 * (tools/ps2eeas_nops.py exempts it). Our C comes out 12 bytes, retail
 * is 16. An older note blamed an assembler load delay.
 */
ASM_FUNC("asm/handwritten/text", func_001F98B0);

ASM_FUNC("asm/handwritten/text", func_001F98C0);

ASM_FUNC("asm/handwritten/text", func_001F98E8);

ASM_FUNC("asm/handwritten/text", func_001F98F8);

ASM_FUNC("asm/handwritten/text", func_001F9908);

ASM_FUNC("asm/handwritten/text", func_001F9938);

ASM_FUNC("asm/handwritten/text", func_001F9968);

ASM_FUNC("asm/handwritten/text", func_001F9978);

ASM_FUNC("asm/handwritten/text", func_001F9988);

ASM_FUNC("asm/handwritten/text", func_001F99B0);

ASM_FUNC("asm/handwritten/text", func_001F99D8);

ASM_FUNC("asm/handwritten/text", func_001F9A00);

ASM_FUNC("asm/handwritten/text", func_001F9A98);

ASM_FUNC("asm/handwritten/text", func_001F9AC0);

/*
 * Not a decompile target: hand-written assembly, same idioms as the
 * func_0020C210/func_0020C230 DMAC block. It keeps the 0x1000D000 base
 * live in $1/$at across all four stores (assembler-reserved, never
 * allocated by GCC), materializes 0x100 with `ori $2,$0,0x100` rather
 * than `addiu`, has a bare `nop` between two stores, and leaves its
 * final value in $3 -- not $2, the ABI return register -- which no
 * compiler would emit for a value-returning function. An earlier
 * revision of this comment treated it as compiler output and filed the
 * diff under the scratch-register-allocation open question; that was a
 * misread.
 */
ASM_FUNC("asm/handwritten/text", func_001F9AF0);

ASM_FUNC("asm/handwritten/text", func_001F9B20);

LINKER_REMNANT("asm/remnants/text", func_001F9B48);

INCLUDE_ASM("asm/nonmatchings/text", func_001F9B50);

ASM_FUNC("asm/handwritten/text", func_001F9B70);

float FastAbsF(float arg0) {
    return __builtin_fabsf(arg0);
}

/*
 * REVERTED to INCLUDE_ASM deliberately, and this one matters beyond
 * these two functions.
 *
 * They are max.s/min.s single-instruction wrappers. Retail is 8 bytes
 * each (max.s scheduled into the jr's delay slot). Written as inline asm
 * this compiler emits 12 bytes -- jr first, then the asm in dead-code
 * position, because inline asm is opaque to its scheduler. So they were
 * never matches; they were 4 bytes LONGER than retail each.
 *
 * A non-matching function that is *longer* than retail shifts every
 * later function in the segment, so these two were silently adding 8
 * bytes of drift to every downstream address reference in text.c --
 * func_001F7B40, for instance, was instruction-for-instruction identical
 * to retail yet reported 1/44 purely because its `jal` target had moved.
 * Reverting them restores the stubs' exact retail bytes and removes that
 * drift.
 *
 * General rule this establishes: a size-mismatched function should be
 * reverted to INCLUDE_ASM, not kept as documented-close C. Byte-diff
 * near-misses of the same size are harmless to keep; longer ones corrupt
 * verification for everything after them. (This is the same failure mode
 * as the old core_text func_00112380 drift, which is why that one was
 * worth fixing rather than tolerating.)
 */
ASM_FUNC("asm/handwritten/text", func_001F9B90);

ASM_FUNC("asm/handwritten/text", func_001F9B98);

ASM_FUNC("asm/handwritten/text", func_001F9BA0);

ASM_FUNC("asm/handwritten/text", func_001F9BB0);

/*
 * Same-size-but-4-over near-miss NOT kept (size mismatch: 12 vs
 * retail's 8): `*(unsigned long long *)arg0 = 0;` (confirmed the
 * correct 128-bit lq/sq type per the 64-bit-field-type rule) compiles
 * to `por v0,zero,zero` materializing the zero into a real register
 * before the `sq`, where retail's `sq zero,0(a0)` uses $0 directly as
 * the store source. `__builtin_memset(arg0, 0, 16)` produces the same
 * extra instruction. Not reachable from source tried so far.
 */
ASM_FUNC("asm/handwritten/text", func_001F9BC0);

ASM_FUNC("asm/handwritten/text", func_001F9BC8);

ASM_FUNC("asm/handwritten/text", func_001F9BD8);

ASM_FUNC("asm/handwritten/text", func_001F9BF0);

ASM_FUNC("asm/handwritten/text", func_001F9C08);

ASM_FUNC("asm/handwritten/text", func_001F9C30);

ASM_FUNC("asm/handwritten/text", func_001F9C48);

ASM_FUNC("asm/handwritten/text", func_001F9C60);

ASM_FUNC("asm/handwritten/text", func_001F9C78);

ASM_FUNC("asm/handwritten/text", func_001F9CA0);

ASM_FUNC("asm/handwritten/text", func_001F9CB8);

ASM_FUNC("asm/handwritten/text", func_001F9CE8);

ASM_FUNC("asm/handwritten/text", func_001F9D10);

ASM_FUNC("asm/handwritten/text", func_001F9D48);

ASM_FUNC("asm/handwritten/text", func_001F9D78);

ASM_FUNC("asm/handwritten/text", func_001F9DC0);

ASM_FUNC("asm/handwritten/text", func_001F9E10);

ASM_FUNC("asm/handwritten/text", func_001F9E58);

ASM_FUNC("asm/handwritten/text", func_001F9EC0);

ASM_FUNC("asm/handwritten/text", func_001F9EE8);

ASM_FUNC("asm/handwritten/text", func_001F9F18);

ASM_FUNC("asm/handwritten/text", func_001F9F30);

ASM_FUNC("asm/handwritten/text", func_001F9F60);

INCLUDE_ASM("asm/nonmatchings/text", func_001F9F90);

INCLUDE_ASM("asm/nonmatchings/text", func_001F9FA8);

INCLUDE_ASM("asm/nonmatchings/text", func_001F9FC0);

ASM_FUNC("asm/handwritten/text", func_001FA058);

ASM_FUNC("asm/handwritten/text", func_001FA168);

ASM_FUNC("asm/handwritten/text", func_001FA190);

ASM_FUNC("asm/handwritten/text", func_001FA1C0);

ASM_FUNC("asm/handwritten/text", func_001FA1F8);

ASM_FUNC("asm/handwritten/text", func_001FA218);

ASM_FUNC("asm/handwritten/text", func_001FA238);

ASM_FUNC("asm/handwritten/text", func_001FA460);

/*
 * Reverted (size mismatch: 28 vs retail's 32). Semantics certain: a
 * plain 3-quadword (48-byte) copy --
 *   void func_001FA480(void *arg0, void *arg1) {
 *       *(unsigned long long *)((char *)arg0 + 0x00) = *(unsigned long long *)((char *)arg1 + 0x00);
 *       *(unsigned long long *)((char *)arg0 + 0x10) = *(unsigned long long *)((char *)arg1 + 0x10);
 *       *(unsigned long long *)((char *)arg0 + 0x20) = *(unsigned long long *)((char *)arg1 + 0x20);
 *   }
 * (128-bit lq/sq per the usual `unsigned long long` rule; a struct-copy
 * spelling gives identical output). Retail loads all 3 quadwords, then
 * stores all 3, then `jr ra` / plain `nop`. This compiler also loads
 * all 3 first, but hoists the LAST store into the `jr`'s delay slot.
 * Update 2026-09-23: hand-written, not a compiler schedule. Retail's
 * first quadword goes through $1 ($at), which gcc's MIPS port never
 * allocates, and its twin func_001FA460 has the same body with a
 * `sqc2 $vf0` in the slot. fastfunc.o is hand-written noreorder code.
 */
ASM_FUNC("asm/handwritten/text", func_001FA480);

ASM_FUNC("asm/handwritten/text", func_001FA4A0);

ASM_FUNC("asm/handwritten/text", func_001FA4F0);

ASM_FUNC("asm/handwritten/text", func_001FA540);

ASM_FUNC("asm/handwritten/text", func_001FA588);

ASM_FUNC("asm/handwritten/text", func_001FA5C8);

ASM_FUNC("asm/handwritten/text", func_001FA648);

ASM_FUNC("asm/handwritten/text", func_001FA6C0);

/*
 * Close but not exact: angle-wrap-to-[-pi,pi] on arg0+arg1 (retail's
 * else-if shape confirmed by the delay-slot second-compare testing the
 * *original* sum, only meaningful when the first branch wasn't taken).
 * Logic and instruction sequence confirmed correct via objdump, but
 * this compiler allocates the sum into $f12 (reusing arg0's register)
 * where retail uses a fresh $f0 -- same scratch-register-allocation-
 * choice open question as elsewhere, now confirmed to apply to FP
 * registers too, not just integer. Tried forcing a fresh local via a
 * separate assignment+accumulate instead of one combined expression;
 * no change. See docs/DECOMP_PROGRESS.md.
 */
ASM_FUNC("asm/handwritten/text", func_001FA748);

/* Same register-allocation-choice issue as func_001FA748 above (its
   arg0-arg1 sibling); logic confirmed identical shape via objdump. */
ASM_FUNC("asm/handwritten/text", func_001FA790);

ASM_FUNC("asm/handwritten/text", func_001FA7D8);

ASM_FUNC("asm/handwritten/text", func_001FA850);

float IntToFloat(int arg0) {
    return (float)arg0;
}

/*
 * Close but not exact: retail's cvt.w.s converts in place (dest fs ==
 * src fs, both $f12, the incoming arg register); this compiler always
 * allocates a fresh dest register for the conversion result. 2/16 bytes
 * differ (just that one instruction's register-field encoding). New,
 * small instance of the scratch-register-allocation-choice open
 * question. Tried an extra (float)(int) round-trip to nudge it toward
 * reusing $f12 -- made it worse (50% mismatch), reverted.
 */
INCLUDE_ASM("asm/nonmatchings/text", func_001FA898);

ASM_FUNC("asm/handwritten/text", func_001FA8A8);

ASM_FUNC("asm/handwritten/text", func_001FA8F0);

ASM_FUNC("asm/handwritten/text", func_001FA9E8);
