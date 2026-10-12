#include "common.h"
#include "structs.h"

extern void func_0012C3B0(void *);

extern void r1_func_0012C468(void *, void *);

extern int func_0012C878(void *, int);

extern void func_0012C380(char *, unsigned int);

extern void func_0012D100(void *);

extern char r1_D_001539E8[];

extern char r1_D_00153A10[];

extern void func_0012D1C8(void *);

typedef struct {
    char pad_00[0x13C];
    int  unk_13C;
    char pad_140[0x150 - 0x140];
    int  unk_150;
    char pad_154[0x174 - 0x154];
    int  unk_174;
    int  unk_178;
    char pad_17C[0x184 - 0x17C];
    int  unk_184;
    char pad_188[0x18C - 0x188];
    int  f1[3];
    int  f2[3];
} State;

extern void func_0012D2C0(void *);

extern int func_0012B678(void *);

extern char r1_D_00153A40[];

extern char r1_D_00153A60[];

extern void r1_func_00129E30();

extern void r1_func_00129F40();

extern char r1_D_00153A80[];

extern void r1_func_00116248_4(void *, char *, int, int) __asm__("r1_func_00116248");

extern void r1_func_0012C468_a(void *, void *) __asm__("r1_func_0012C468");

/* rac1-decomp src/core/00125630.c: func_001292E0 */
/* quantMatrixExtension (libmpeg.a:mpc.o): ISO/IEC 13818-2 6.2.3.2
 * quant_matrix_extension(). Unlike the reference decoder's 64-coefficient
 * software copy loops, this hardware-assisted port just flags the IPU to
 * pull the following intra/non-intra matrix straight off the bitstream
 * (waitIpuIdle, sendIpuCommand(0x50000000 / 0x58000000), waitIpuIdle) when
 * load_intra/non_intra_quantizer_matrix is set, storing the flag at
 * +0x840/+0x844 either way. The chroma variants aren't hardware-supported:
 * setting either just logs a warning through r1_func_0012C468.
 */
void func_0012D100(void *arg0) {
    char *s = (char *)arg0;
    int flag;

    flag = func_0012C878(s, 1);        /* load_intra_quantizer_matrix */
    *(int *)(s + 0x840) = flag;
    if (flag != 0) {
        func_0012C3B0(s);
        func_0012C380(s, 0x50000000);
        func_0012C3B0(s);
    }

    flag = func_0012C878(s, 1);        /* load_non_intra_quantizer_matrix */
    *(int *)(s + 0x844) = flag;
    if (flag != 0) {
        func_0012C3B0(s);
        func_0012C380(s, 0x58000000);
        func_0012C3B0(s);
    }

    if (func_0012C878(s, 1) != 0) {    /* load_chroma_intra_quantizer_matrix */
        r1_func_0012C468(s, r1_D_001539E8);
    }

    if (func_0012C878(s, 1) != 0) {    /* load_chroma_non_intra_quantizer_matrix */
        r1_func_0012C468(s, r1_D_00153A10);
    }
}

/* rac1-decomp src/core/00125630.c: func_001293A8 */
void func_0012D1C8(void *arg0) {
    State *s = (State *)arg0;
    int count;
    int i;

    if (s->unk_13C != 0) {
        if (s->unk_184 == 0) {
            count = 1;
        } else {
            count = s->unk_178 != 0 ? 3 : 2;
        }
    } else {
        if (s->unk_174 != 3) {
            count = 1;
        } else {
            count = s->unk_184 != 0 ? 3 : 2;
        }
    }

    for (i = 0; i < count; i++) {
        s->f1[i] = func_0012C878(s, 0x10);
        func_0012C878(s, 1);
        s->f2[i] = func_0012C878(s, 0x10);
        func_0012C878(s, 1);
    }
}

/* rac1-decomp src/core/00125630.c: func_001294A0 */
/* copyrightExtension (libmpeg.a:mpc.o) -- ISO/IEC 13818-2 6.2.3.6: skip the
 * copyright_extension() fields with func_0012C878 (nextBit, "get n bits").
 * Reference decoder (mpeg2decode gethdr.c copyright_extension()) assigns
 * each field to a local only used by its VERBOSE printf, which this port
 * drops entirely, so every nextBit's result is a dead store the compiler
 * elides -- the sequence is bare `nextBit(s, width)` calls, and the final
 * one, its value discarded, becomes 2.9-ee's bare tail jump.
 */
void func_0012D2C0(void *arg0) {
    func_0012C878(arg0, 1);   /* copyright_flag */
    func_0012C878(arg0, 8);   /* copyright_identifier */
    func_0012C878(arg0, 1);   /* original_or_copy */
    func_0012C878(arg0, 7);   /* reserved */
    func_0012C878(arg0, 1);   /* marker_bit */
    func_0012C878(arg0, 20);  /* copyright_number_1 */
    func_0012C878(arg0, 1);   /* marker_bit */
    func_0012C878(arg0, 22);  /* copyright_number_2 */
    func_0012C878(arg0, 1);   /* marker_bit */
    func_0012C878(arg0, 22);  /* copyright_number_3 */
}

/* rac1-decomp src/core/00125630.c: func_00129530 */
/*
 * Decoder state step: if the picture state (s+0x174) is 3 and s+0x120 is
 * set, report r1_D_00153A40 through r1_func_0012C468, clear s+0x120 and re-read
 * the state; pick the target pointer by state (3 -> +0x1C0, 1 -> +0x1D0,
 * 2 -> +0x1E0, anything else -> +0x1C0 plus a r1_D_00153A60 report; the
 * field is a POINTER, the target of the final +0x28 write); then
 * r = func_0012B678(s), and if r, target->+0x28 = 1. Exact under both
 * compilers.
 *
 * It is a `switch`, with the case bodies in the order 3, 1, 2, default.
 * GCC's decision tree is fixed (==2, <3, ==1 / ==3), but the body that
 * follows the dispatch is the first case in source order, and jump.c
 * then turns `beq 3,case3; b default` into retail's `bne 3,default` plus
 * fall-through (the nested-if decode was 200 against 204). The state is
 * re-read into the local after the report, as 2.95.3 needs; switching on
 * the memory re-read is exact under 2.9-ee too.
 */
int func_0012D350(void *arg0) {
    char *s = arg0;
    int state = *(int *)(s + 0x174);
    char *target;
    int r;

    if (state == 3 && *(int *)(s + 0x120) != 0) {
        r1_func_0012C468(s, r1_D_00153A40);
        *(int *)(s + 0x120) = 0;
        state = *(int *)(s + 0x174);
    }
    switch (state) {
    case 3:
        target = *(char **)(s + 0x1C0);
        break;
    case 1:
        target = *(char **)(s + 0x1D0);
        break;
    case 2:
        target = *(char **)(s + 0x1E0);
        break;
    default:
        target = *(char **)(s + 0x1C0);
        r1_func_0012C468(s, r1_D_00153A60);
        break;
    }
    r = func_0012B678(s);
    if (r != 0) {
        *(int *)(target + 0x28) = 1;
    }
    return r;
}

/* rac1-decomp src/core/00125630.c: func_00129600 */
void func_0012D420(void *arg0, int arg1, int arg2) {
    char *s = (char *)arg0;
    int v, w;

    if (arg2 != 0) {
        if (*(int *)(s + 0x174) == 3) {
            if (*(int *)(s + 0x150) == 3) {
                v = *(int *)(s + 0x1C4);
            } else {
                v = *(int *)(s + 0x1B8);
            }
            r1_func_00129E30(s, v, arg1 - 1);
        } else {
            if (*(int *)(s + 0x150) == 3) {
                v = *(int *)(s + 0x1D4);
                w = *(int *)(s + 0x1E4);
            } else {
                v = *(int *)(s + 0x1C8);
                w = *(int *)(s + 0x1D8);
            }
            r1_func_00129F40(s, v, w, arg1 - 1);
        }
    }
    if (*(int *)(s + 0xF8) == 1) {
        *(int *)(s + 0xF8) = 2;
    }
}

INCLUDE_ASM("asm/core", func_0012D4B0);

/* rac1-decomp src/core/00125630.c: func_00129948 */
/* Does the request at arg1 fit the heap described by arg0? A sized
   request (+0xE0 non-zero) has to fit both the byte budget at +0xDC and
   the entry budget at +0xE0; an unsized one has to fit width * height
   against +0xE4. On a refusal, format the two figures into the message
   at r1_D_00153A80 and report it. Returns whether it fits. */
int func_0012D768(void *arg0, void *arg1) {
    char buf[0x100];
    char *h = (char *)arg0;
    char *q = (char *)arg1;
    int e = *(int *)(h + 0xE0);
    int ok;

    if (e != 0) {
        ok = *(int *)(h + 0xDC) >= *(int *)(q + 0x4) &&
             e >= *(int *)(q + 0x8);
    } else {
        ok = *(int *)(h + 0xE4) >= *(int *)(q + 0xC) * *(int *)(q + 0x10);
    }
    if (ok == 0) {
        r1_func_00116248_4(buf, r1_D_00153A80, *(int *)(q + 0x4),
                        *(int *)(q + 0x8));
        r1_func_0012C468_a(arg0, buf);
    }
    return ok;
}
