#include "common.h"
#include "structs.h"

typedef struct {
    char unk_00[4];
    int  unk_04;
    int  unk_08;
    char *unk_0C;
    char unk_10[0x320];
} Ent330;

extern Ent330 r1_D_0015B640[];

extern int func_00128D58(int);

extern int func_00128578(int);

extern void r1_func_00119288(void *, void *);

extern void *func_00128DB0(int);

extern void r1_func_00119288(void *a, void *b);

extern int func_00128E18(int);

extern int r1_D_00132ED8[];

extern int func_00128E98(unsigned char *, int *);

/* rac1-decomp src/core/001236F0.c: func_00125020 */
/* scePad2LinkDriver(port): opens the link driver for `port` and, on
   success, records the handle in its table entry. The failure path needs
   its own early return: with one shared `return t` the compiler moves the
   result copy to the end of the function. */
int func_00128D58(int arg0) {
    int t = func_00128578(arg0);
    if (t < 0) {
        return t;
    }
    r1_D_0015B640[arg0].unk_08 = t;
    r1_D_0015B640[arg0].unk_04 = 1;
    return t;
}

/* rac1-decomp src/core/001236F0.c: func_00125078 */
/*
 * Close, not exact (3/100), same size. Logic confirmed: take entry
 * arg0 of the 0x330-stride table r1_D_0015B640, follow its +0xC pointer,
 * record that pointer and pointer+0x80 in a two-slot stack array, call
 * r1_func_00119288(p, p+0x100), then return whichever slot has the smaller
 * +0x7C field -- retail indexes the array with the `slt` result
 * directly, which plain C reproduces.
 *
 * The three differing bytes were one instruction: retail forms the entry
 * address as `addu $2,$2,$4` (base, index) where we emitted
 * `addu $a0,$a0,$v0` (index, base). Writing the addition the other way
 * round changes nothing -- GCC canonicalises the PLUS -- and this was
 * recorded here as "the allocator's destination choice, not operand
 * order", which was wrong. It is operand order, and it is steerable:
 * give the table a real element type and index it. See Ent330 above.
 */
void *func_00128DB0(int arg0) {
    char *p = r1_D_0015B640[arg0].unk_0C;
    char *slot[2];

    slot[0] = p;
    slot[1] = p + 0x80;
    r1_func_00119288(p, p + 0x100);
    return slot[*(int *)(slot[0] + 0x7C) < *(int *)(slot[1] + 0x7C)];
}

/* rac1-decomp src/core/001236F0.c: func_001250E0 */
/* Has entry arg0's current pair changed? Follow entry arg0 of the
   0x330-stride table to its buffer p; the pair is p+0x7C and p+0x80+0x7C.
   If the first is zero, or both equal the cached pair r1_D_00132ED8[0..1],
   report 0; otherwise cache them and report 1. The two-slot stack array
   is retail's (the same idiom as func_00128DB0). Exact under this file's
   2.9-ee; under 2.95.3 it was a word long. */
/* The old note's decode, exact since 001236F0.c builds with 2.9-ee. */
int func_00128E18(int arg0) {
    char *p = r1_D_0015B640[arg0].unk_0C;
    char *slot[2];

    slot[0] = p;
    slot[1] = p + 0x80;
    if (*(int *)(p + 0x7C) == 0 ||
        (r1_D_00132ED8[0] == *(int *)(p + 0x7C) &&
         r1_D_00132ED8[1] == *(int *)(slot[1] + 0x7C))) {
        return 0;
    }
    r1_D_00132ED8[0] = *(int *)(slot[0] + 0x7C);
    r1_D_00132ED8[1] = *(int *)(slot[1] + 0x7C);
    return 1;
}

/* rac1-decomp src/core/001236F0.c: func_00125160 */
/* Expand a 40-bit mask into 40 four-word slots. For each set bit, record
   slot type 1 plus the running (a, b) pair; entries 0x10..0x1F and
   0x23..0x26 are "wide" (size 8) and only advance a, everything else is
   size 1 and advances b, rolling a over every eighth. Clear bits zero
   the slot's type and both counters. Initialising b before the loop
   sets i gives i the shorter live range, so this file's compiler
   (2.9-ee) allocates it first, as retail. */
int func_00128E98(unsigned char *src, int *out) {
    int bit;
    int a;
    int i;
    int b;

    bit = 0;
    a = 0;
    b = 0;
    for (i = 0; i < 0x28; i++) {
        if (((*src >> bit) & 1) != 0) {
            out[0] = 1;
            out[2] = a;
            out[3] = b;
            if ((i >= 0x10 && i < 0x20) || (i >= 0x23 && i < 0x27)) {
                out[1] = 8;
                a++;
            } else {
                b++;
                out[1] = 1;
                if ((b & 7) == 0) {
                    a++;
                    b = 0;
                }
            }
        } else {
            out[0] = 0;
            out[2] = 0;
            out[3] = 0;
        }
        bit++;
        if ((bit & 7) == 0) {
            src++;
            bit = 0;
        }
        out += 4;
    }
    return 1;
}

INCLUDE_ASM("asm/core", func_00128F48);

INCLUDE_ASM("asm/core", func_00128FD0);

INCLUDE_ASM("asm/core", func_001290BC);

INCLUDE_ASM("asm/core", func_00129120);

INCLUDE_ASM("asm/core", func_00129160);

INCLUDE_ASM("asm/core", func_001291A0);

INCLUDE_ASM("asm/core", func_00129218);

INCLUDE_ASM("asm/core", func_001292C0);

INCLUDE_ASM("asm/core", func_00129368);

INCLUDE_ASM("asm/core", func_00129410);
