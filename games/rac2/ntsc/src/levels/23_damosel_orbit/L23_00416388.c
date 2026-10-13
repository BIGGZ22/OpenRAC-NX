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

INCLUDE_ASM("asm/levels", func_L23_00418F30);

INCLUDE_ASM("asm/levels", func_L23_00418FD0);

/* Shifts a 64-bit value right by a variable count and keeps its lowest bit. */
s32 func_L23_00419070(u64 arg0, s32 arg1) { return (s32)((arg0 >> arg1) & 1); }

INCLUDE_ASM("asm/levels", func_L23_00419088);

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

INCLUDE_ASM("asm/levels", func_L23_0041B540);

/* Clears three words of the state at +0x68. */
void func_L23_0041B5B0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(s32 *)(state + 0x5CC) = 0;
    *(s32 *)(state + 0x5C8) = 0;
    *(s32 *)(state + 0x5DC) = 0;
}

INCLUDE_ASM("asm/levels", func_L23_0041B5C8);

INCLUDE_ASM("asm/levels", func_L23_0041B668);

INCLUDE_ASM("asm/levels", func_L23_0041B6D8);

INCLUDE_ASM("asm/levels", func_L23_0041B940);

INCLUDE_ASM("asm/levels", func_L23_0041B960);

INCLUDE_ASM("asm/levels", func_L23_0041B968);

INCLUDE_ASM("asm/levels", func_L23_0041B9F0);

INCLUDE_ASM("asm/levels", func_L23_0041BE70);

INCLUDE_ASM("asm/levels", func_L23_0041BF18);

INCLUDE_ASM("asm/levels", func_L23_0041C040);

INCLUDE_ASM("asm/levels", func_L23_0041C0B0);

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

INCLUDE_ASM("asm/levels", func_L23_0041D930);

INCLUDE_ASM("asm/levels", func_L23_0041D9B0);

INCLUDE_ASM("asm/levels", func_L23_0041DA48);

INCLUDE_ASM("asm/levels", func_L23_0041E220);

INCLUDE_ASM("asm/levels", func_L23_0041E290);

