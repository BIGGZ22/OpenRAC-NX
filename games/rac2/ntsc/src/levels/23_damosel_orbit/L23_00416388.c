#include "include_asm.h"

typedef signed short s16;

typedef unsigned short u16;

typedef signed int s32;

typedef unsigned long long u64;

INCLUDE_ASM("asm/levels", func_L23_00416388);

INCLUDE_ASM("asm/levels", func_L23_004177F8);

extern void func_00282948(char *);

extern void func_00282978(char *);

/* Refreshes two measured state pairs and advances the two counters under their limits. */
void func_L23_004180F0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    func_00282948(state + 0x348);
    func_00282978(state + 0x34C);
    if (*(u16 *)(arg0 + 0x96) <= 0xFFFE)
        *(u16 *)(arg0 + 0x96) = *(u16 *)(arg0 + 0x96) + 1;
    if (*(s32 *)(state + 0x5AC) <= 0xFFFE)
        *(s32 *)(state + 0x5AC) = *(s32 *)(state + 0x5AC) + 1;
}

INCLUDE_ASM("asm/levels", func_L23_00418160);

INCLUDE_ASM("asm/levels", func_L23_004183F0);

INCLUDE_ASM("asm/levels", func_L23_00418650);

INCLUDE_ASM("asm/levels", func_L23_004186D0);

INCLUDE_ASM("asm/levels", func_L23_00418BC8);

INCLUDE_ASM("asm/levels", func_L23_00418DC0);

extern s32 func_L23_00419CA0(s32 arg0);

/* Collects the set bits of the mask whose index the shared predicate accepts. */
s32 func_L23_00418F30(char *arg0, u64 arg1, s32 *arg2, s32 arg3) {
    s32 count = 0;
    s32 *out = arg2;
    s32 i;

    for (i = 0; i < 0x40; i++) {
        if ((arg1 & 1) != 0 && (arg3 == 0 || func_L23_00419CA0(i) != 0)) {
            *out = i;
            count++;
            out++;
        }
        arg1 >>= 1;
    }
    return count;
}

extern s32 func_L23_00419070(u64, s32);

extern int *D_L23_001FF300[];

extern char D_L23_002595C8[];

/* Returns 0 when bit arg of the slot's first mask is set; otherwise tests the second mask and
   returns the slot's count minus one. */
s32 func_L23_00418FD0(char *arg0, s32 index, s32 arg) {
    s32 *slots = (s32 *)(*(char **)(arg0 + 0x68) + 0x230);
    s32 *slot = &slots[index];
    s32 v;
    s32 n;

    if (func_L23_00419070(*(u64 *)(D_L23_002595C8 + (*slot << 4)), arg) != 0)
        return 0;
    v = *slot;
    n = *D_L23_001FF300[v] - 1;
    func_L23_00419070(*(u64 *)(D_L23_002595C8 + (v << 4) + 8), arg);
    return n;
}

/* Shifts a 64-bit value right by a variable count and keeps its lowest bit. */
s32 func_L23_00419070(u64 arg0, s32 arg1) { return (s32)((arg0 >> arg1) & 1); }

/* Advances the three-slot ring at +0x398, storing the previous value of +0x394. */
void func_L23_00419088(char *arg0, s16 arg1) {
    char *p = *(char **)(arg0 + 0x68);
    s16 n = *(s16 *)(p + 0x39E);
    u16 old = *(u16 *)(p + 0x394);
    int off;
    int r;

    r = (n + 1) % 3;
    off = r * 2;
    *(s16 *)(p + 0x39E) = r;
    *(s16 *)(p + off + 0x398) = old;
    *(s16 *)(p + 0x394) = arg1;
}

INCLUDE_ASM("asm/levels", func_L23_004190C8);

INCLUDE_ASM("asm/levels", func_L23_004196B8);

INCLUDE_ASM("asm/levels", func_L23_00419970);

/* One when exactly one bit of the mask is set and the shared predicate rejects its index. */
s32 func_L23_00419CE0(u64 arg0) {
    s32 count = 0;
    s32 found = 0;
    s32 i = 0;

    do {
        if (arg0 & 1) {
            count++;
            if (count >= 2)
                return 0;
            found = i;
        }
        i++;
        arg0 >>= 1;
    } while (i < 0x40);

    if (count == 0)
        return 0;
    if (count == 1) {
        if (func_L23_00419CA0(found) == 0)
            return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L23_00419D60);

INCLUDE_ASM("asm/levels", func_L23_0041A038);

INCLUDE_ASM("asm/levels", func_L23_0041A3B8);

INCLUDE_ASM("asm/levels", func_L23_0041A7A0);

INCLUDE_ASM("asm/levels", func_L23_0041AFE8);

INCLUDE_ASM("asm/levels", func_L23_0041B188);

extern void func_L00_0030C930(void);

/* Calls the measured shared helper and reports success. */
s32 func_L23_0041B478(void) {
    func_L00_0030C930();
    return 1;
}

extern float func_00282C48(float);

/* Relatches the first measure when the incoming one has moved at least a tenth away. */
void func_L23_0041B540(char *arg0, float *arg1) {
    char *state = *(char **)(arg0 + 0x68);

    if (!(func_00282C48(*(float *)(state + 0x5D0) - *arg1) < 0.1f)) {
        *(float *)(state + 0x5D0) = *arg1;
        *(s32 *)(state + 0x5CC) = 0;
        *(s32 *)(state + 0x5DC) = 0;
    }
}

/* Clears three words of the state at +0x68. */
void func_L23_0041B5B0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(s32 *)(state + 0x5CC) = 0;
    *(s32 *)(state + 0x5C8) = 0;
    *(s32 *)(state + 0x5DC) = 0;
}

/* Returns the new value in $f0. Declared void, the call leaves $f0 free and the measure
 * reloaded right after it takes $f0 where retail has $f1. */
extern float func_002AA548(float *, float *, float, float, float, float);

/* Springs the second measure toward its target, doubles the word at +0x74, and latches the
 * measure once it is within a tenth of the target. */
void func_L23_0041B5C8(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    if (*(s32 *)(state + 0x5D8) != 0)
        return;
    func_002AA548((float *)(state + 0x588), (float *)(state + 0x58C), *(float *)(state + 0x5D4),
                  0.016666668f, 0.016666668f, 0.40833336f);
    *(float *)(state + 0x74) *= 2.0f;
    if (func_00282C48(*(float *)(state + 0x588) - *(float *)(state + 0x5D4)) < 0.1f) {
        *(s32 *)(state + 0x5D8) = 1;
        *(s32 *)(state + 0x58C) = 0;
    }
}

/* The same relatch as the first measure, for the second one and its own flags. */
void func_L23_0041B668(char *arg0, float *arg1) {
    char *state = *(char **)(arg0 + 0x68);

    if (!(func_00282C48(*(float *)(state + 0x5D4) - *arg1) < 0.1f)) {
        *(float *)(state + 0x5D4) = *arg1;
        *(s32 *)(state + 0x58C) = 0;
        *(s32 *)(state + 0x5D8) = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L23_0041B6D8);

INCLUDE_ASM("asm/levels", func_L23_0041B940);

INCLUDE_ASM("asm/levels", func_L23_0041B9F0);

INCLUDE_ASM("asm/levels", func_L23_0041C0E8);

INCLUDE_ASM("asm/levels", func_L23_0041C578);

INCLUDE_ASM("asm/levels", func_L23_0041C6B0);

/* func_L23_0041C7D8 is matched in L23_00416388-g8.c */

/* The same table and counters func_L23_0041CCC0 clears below, declared the same way. */
extern s32 D_L23_001B3744 __attribute__((sda));

extern s32 D_L23_001B3738[3] __attribute__((sda));

extern s32 D_L23_00259050[3][60];

typedef int TI128 __attribute__((mode(TI)));

extern char *func_L22_003FB1F8(char *, TI128 *, s32, float);

extern char *func_L22_003FC780(char *, TI128 *, s32, float);

extern char *func_L22_003FD1E8(char *, TI128 *, s32, float);

/* Spawns an object of kind TYPE at POS in the first free slot of its row, counts it, and gives
 * it the position as it was on entry. Returns the slot index, or -1 when the row is full.
 * The do-while (0) around the first copy leaves loop notes after it, which the schedulers treat
 * as a barrier (the lever of func_00124818): the argument copies and the 16-byte copy stay
 * ahead of the search, so $a0 is free for the 0xF0 factor and the object goes to $t0. Wrapping
 * the search instead doubles its reference weights, and local-alloc then gives the first word
 * $v0 ahead of the table address. The vector is kept in memory (a TI128 local is kept in a
 * register pair across the calls), and the table pointer is read back by D[type][i]. */
s32 func_L23_0041C8C0(char *obj, TI128 *pos, s32 type, s32 arg3) {
    float saved[4] __attribute__((aligned(16)));
    char *slot;
    s32 i;

    do {
        *(TI128 *)saved = *pos;
    } while (0);
    for (i = 0; i < 50; i++) {
        if (D_L23_00259050[type][i] == 0)
            break;
        if (i == 49) {
            i = -1;
            break;
        }
    }
    if (i != -1) {
        switch (type) {
        case 0:
            D_L23_00259050[type][i] = (s32)func_L22_003FB1F8(obj, pos, arg3, 4.0f);
            break;
        case 1:
            D_L23_00259050[type][i] = (s32)func_L22_003FC780(obj, pos, arg3, 3.65f);
            break;
        case 2:
            D_L23_00259050[type][i] = (s32)func_L22_003FD1E8(obj, pos, arg3, 3.65f);
            break;
        }
        slot = (char *)D_L23_00259050[type][i];
        if (slot != 0) {
            D_L23_001B3744++;
            D_L23_001B3738[type]++;
            *(TI128 *)(slot + 0x10) = *(TI128 *)saved;
        }
    }
    return i;
}

INCLUDE_ASM("asm/levels", func_L23_0041CA58);

/* The two sda declarations give retail's bare-symbol forms in this -G0 unit: the store through
 * $at and the unsplit address pair of the small array. */
extern s32 D_L23_001B3744 __attribute__((sda));

extern s32 D_L23_001B3738[3] __attribute__((sda));

extern s32 D_L23_001B1FA0[4];

extern s32 D_L23_00259050[3][60];

/* Clears the three collected rows and their counts, then latches the two state counters.
 * The row pointer is taken after the count store: taken first, it trades the registers of
 * the 0xF0 factor and the count array; and an explicit row (rather than D[i][j] in the inner
 * loop) keeps the multiply ahead of the counter increment, so the counter stays in place. */
void func_L23_0041CCC0(char *arg0) {
    char *st = *(char **)(arg0 + 0x68);
    s32 i, j;

    D_L23_001B3744 = 0;
    for (i = 0; i < 3; i++) {
        s32 *row;

        D_L23_001B3738[i] = 0;
        row = D_L23_00259050[i];
        for (j = 0x31; j >= 0; j--)
            row[j] = 0;
    }
    D_L23_001B1FA0[3] = *(s16 *)(st + 0x2FE);
    D_L23_001B1FA0[2] = *(s16 *)(st + 0x2FA);
    D_L23_001B1FA0[0] = 0;
    D_L23_001B1FA0[1] = 0;
}

INCLUDE_ASM("asm/levels", func_L23_0041CD58);

INCLUDE_ASM("asm/levels", func_L23_0041D0A0);

/* func_L23_0041D1F0 is matched in L23_0041D1F0-g8.c (built with -fno-gcse). */

INCLUDE_ASM("asm/levels", func_L23_0041D398);

INCLUDE_ASM("asm/levels", func_L23_0041DA48);

/* A three-label switch, the third one sharing the default's body (the recipe of
 * func_L22_003A65E0): with only two cases cc1 emits a comparison chain, and the third label is
 * what makes it build the decision tree rooted on 0xE3B. */
float func_L23_0041E220(char *arg0) {
    switch (*(s16 *)(arg0 + 0xAA)) {
    case 0xE3A:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x2C);
    case 0xE3B:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x44);
    case 0xE3C:
    default:
        return *(float *)(*(char **)(arg0 + 0x68) + 0x44);
    }
}
