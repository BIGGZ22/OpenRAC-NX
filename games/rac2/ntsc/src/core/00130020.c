#include "common.h"
#include "structs.h"

extern void r1_func_0012C278();

extern int func_00130020(void *);

extern void r1_func_0012C278(void *);

extern void r1_func_00127378(int arg0);

extern void func_00130088(void *arg0);

extern void func_00130118(void *);

extern int r1_func_0011D960(void);

extern void r1_func_0011D9A8(void);

extern void func_00130178(void *arg0);

extern void r1_func_0012CF98(int, int);

extern char r1_D_00153BD8[];

/* Unprototyped: r1_func_0011A6C8 is a varargs definition (blocked as such),
   but calling one is fine -- only defining one needs stdarg.h. */
extern void r1_func_0011A6C8();

extern void func_00130240(void *);

extern void func_00130288(void *, void *);

/* _errMessage (func_00130250) sprintf's its format and passes the
   format's arguments on untouched in $a1-$a3, so it is called here
   without a prototype, with the value the "%08x" reports. */
extern void func_00130250();

extern void r1_func_00116248(void *);

/* rac1-decomp src/core/0012AC80.c: func_0012C200 */
int func_00130020(void *arg0) {
    Wrapper *w = (Wrapper *)arg0;
    Obj40 *inner = w->obj;
    int ret = 0;

    if (inner->unk004 != 0 && inner->unk008 != 0) {
        r1_func_0012C278((void *)inner);
        w->unk08 = inner->unk118 - inner->unk0AC;
        inner->unk004 = 0;
        ret = 1;
    }
    return ret;
}

/* rac1-decomp src/core/0012AC80.c: func_0012C268 */
/* Tail call: the constant argument setup lands in the jump's delay slot,
   so the field store precedes it. r1_func_00127378 is defined above. */
void func_00130088(void *arg0) {
    *(int *)((char *)arg0 + 0x848) = 0;
    r1_func_00127378(1);
}

INCLUDE_ASM("asm/core", func_00130098);

/* rac1-decomp src/core/0012AC80.c: func_0012C2F8 */
/*
 * Point the object's four scratchpad pointers at 0x70000000 and clear a
 * flag. Exact under 2.9-ee. Under 2.95.3 it was 2 of 24 words off:
 * that compiler saved $ra before $s1 in the prologue, where retail (and
 * 2.9-ee) saves $s1 first -- the prologue-order residual of the core
 * spill rewrite, not a source question.
 *
 * What does matter: `int a = 0x70000000;` must be written
 * BEFORE the call. An earlier round reverted this at 8 bytes short
 * having tried binding the constants to locals declared AFTER the call,
 * which changes nothing because gcc folds them straight back into the
 * stores. Declared before the call, the pseudo's live range crosses the
 * call, so the allocator gives it a CALLEE-SAVED register and the
 * function pays retail's sd/ld $s1 pair -- 8 bytes. gcc still
 * rematerialises the `lui` after the call, exactly as retail does, so
 * the only trace of the earlier definition is the register class.
 *
 * That is the general point: a constant hoisted above a call does not
 * survive as a value (constant propagation puts it back), but it does
 * survive as a register-class decision. Where retail spends a
 * callee-saved register on something that looks like it needs no
 * register at all, the source defined it before the call.
 */
void func_00130118(void *arg0) {
    char *p = (char *)arg0;
    int a = 0x70000000;

    r1_func_00127378(1);
    *(int *)(p + 0x590) = a;
    *(int *)(p + 0x594) = 0x70001800;
    *(int *)(p + 0x6D0) = 0x70001B00;
    *(int *)(p + 0x6D4) = 0x70003300;
    *(int *)(p + 0x810) = 0;
}

/* rac1-decomp src/core/0012AC80.c: func_0012C358 */
/*
 * _clearEach (libmpeg.a:init.o): per-stream hardware reset shared by
 * sceMpegInit's shape -- flag the bitstream state "in use" (+0x818 = 1),
 * clear its slice-address latch (+0x1B0), then the same SIF-flag toggle
 * (0x1000F590 set/cleared around bit 0x10000, re-reading 0x1000F520 fresh
 * each time) and three DMA channel registers zeroed (0x1000B000/B400/D400),
 * gated IOP kick via r1_func_0011D960/r1_func_0011D9A8, three more registers
 * zeroed (0x1000B020/B420/D420), IPU_CTRL (0x10002010) set to 0x40000000,
 * and a tail call into r1_func_0012CF98(0, 0).
 */
void func_00130178(void *arg0) {
    char *s = (char *)arg0;
    int r;

    *(int *)(s + 0x818) = 1;
    *(int *)(s + 0x1B0) = 0;
    r = r1_func_0011D960();

    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 | 0x10000;
    *(volatile unsigned int *)0x1000B000 = 0;
    *(volatile unsigned int *)0x1000B400 = 0;
    *(volatile unsigned int *)0x1000D400 = 0;
    *(volatile unsigned int *)0x1000F590 =
        *(volatile unsigned int *)0x1000F520 & ~0x10000;

    if (r != 0) {
        r1_func_0011D9A8();
    }

    *(volatile unsigned int *)0x1000B020 = 0;
    *(volatile unsigned int *)0x1000B420 = 0;
    *(volatile unsigned int *)0x1000D420 = 0;
    *(volatile unsigned int *)0x10002010 = 0x40000000;
    r1_func_0012CF98(0, 0);
}

/* rac1-decomp src/core/0012AC80.c: func_0012C420 */
void func_00130240(void *arg0) {
    r1_func_0011A6C8(r1_D_00153BD8, arg0);
}

/* rac1-decomp src/core/0012AC80.c: func_0012C430 */
void func_00130250(void *arg0) {
    char buf[0x100];
    r1_func_00116248(buf);
    func_00130288(arg0, buf);
}
