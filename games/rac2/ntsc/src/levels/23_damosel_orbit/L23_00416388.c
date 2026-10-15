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

INCLUDE_ASM("asm/levels", func_L23_00418FD0);

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

INCLUDE_ASM("asm/levels", func_L23_00419CA0);

INCLUDE_ASM("asm/levels", func_L23_00419CE0);

INCLUDE_ASM("asm/levels", func_L23_00419D60);

INCLUDE_ASM("asm/levels", func_L23_0041A038);

INCLUDE_ASM("asm/levels", func_L23_0041A3B8);

INCLUDE_ASM("asm/levels", func_L23_0041A648);

INCLUDE_ASM("asm/levels", func_L23_0041A7A0);

INCLUDE_ASM("asm/levels", func_L23_0041AFE8);

INCLUDE_ASM("asm/levels", func_L23_0041B188);

extern void func_L00_0030C930(void);

/* Calls the measured shared helper and reports success. */
s32 func_L23_0041B478(void) {
    func_L00_0030C930();
    return 1;
}

INCLUDE_ASM("asm/levels", func_L23_0041B498);

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

INCLUDE_ASM("asm/levels", func_L23_0041B5C8);

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

INCLUDE_ASM("asm/levels", func_L23_0041B968);

INCLUDE_ASM("asm/levels", func_L23_0041B9F0);

INCLUDE_ASM("asm/levels", func_L23_0041BE70);

INCLUDE_ASM("asm/levels", func_L23_0041BF18);

INCLUDE_ASM("asm/levels", func_L23_0041C0E8);

INCLUDE_ASM("asm/levels", func_L23_0041C578);

INCLUDE_ASM("asm/levels", func_L23_0041C6B0);

INCLUDE_ASM("asm/levels", func_L23_0041C7D8);

INCLUDE_ASM("asm/levels", func_L23_0041C8C0);

INCLUDE_ASM("asm/levels", func_L23_0041CA58);

INCLUDE_ASM("asm/levels", func_L23_0041CCC0);

INCLUDE_ASM("asm/levels", func_L23_0041CD58);

INCLUDE_ASM("asm/levels", func_L23_0041D0A0);

INCLUDE_ASM("asm/levels", func_L23_0041D1F0);

INCLUDE_ASM("asm/levels", func_L23_0041D398);

INCLUDE_ASM("asm/levels", func_L23_0041D9B0);

INCLUDE_ASM("asm/levels", func_L23_0041DA48);

INCLUDE_ASM("asm/levels", func_L23_0041E220);

INCLUDE_ASM("asm/levels", func_L23_0041E290);
