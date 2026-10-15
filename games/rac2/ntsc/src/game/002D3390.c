#include "include_asm.h"

/*
 * Disassembled at 0x002cd220
 * Signature: int FUN_002cd220(void)
 * Behavior:
 *   - Saves RA on stack
 *   - Calls func_0027C540(0)
 *   - Calls func_0029C138()
 *   - Calls func_0027C660()
 *   - Restores RA from stack
 *   - Returns 0
 */
extern void func_0027C540(long);

extern void func_0027C660(void);

void func_0027C540(long);

void func_0027C660(void);

/* 
 * Function: func_002D2EE0
 * 
 * Disassembly summary:
 * - Saves return address on stack
 * - Calls func_0027C540(0) (with zero argument)
 * - Calls func_0029C7A0()
 * - Calls func_0027C660()
 * - Restores return address
 * - Returns 0
 *
 * Notes:
 * - The function follows standard MIPS ABI prologue/epilogue.
 * - Delay slots are filled with NOP or argument setup as seen in disassembly.
 * - No local variables used.
 * - All external function addresses come from the assigned externals list.
 */

extern void func_0027C540(long arg0);

/* External functions declared with exact aliases from task.externals */
extern void func_0027C540(long param_1);

extern void func_0029C960(void);

extern char F7cb419c1_D_001F2840[];

extern int F7cb419c1_FUN_002DE618(int arg);

/* Candidate function: FUN_002d3390 */
unsigned long long func_002D3390(void)
{
    func_0027C540(0);
    func_0029C960();
    func_0027C660();
    return 0;
}

extern char *D_001A8CFC[] __attribute__((sda));
extern float ref_L00_002D33C0_00[];
extern void ref_L00_002D33C0_01(unsigned char *, float *);

int func_002D33C0(void)
{
    if (D_001A8CFC[0] != 0)
        ref_L00_002D33C0_01((unsigned char *)(D_001A8CFC[0] + 0x3EDB8), ref_L00_002D33C0_00);
    return 0;
}

INCLUDE_ASM("asm/game", func_002D3400);

extern void func_0029C5A0(void);

int func_002D3488(void) {
    func_0027C540(0);
    func_0029C5A0();
    func_0027C660();
    return 0;
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void func_003435B0(char *, int);
extern void func_00343538(char *, char *);
extern void func_00343580(char *);
extern void func_003435A0(char *, char *);
extern char D_00259F58[];
extern char D_0025D0E0[];

int func_002D34B8(void)
{
    if (D_001A8CFC[0] != 0) {
        func_003435B0(D_001A8CFC[0] + 0x3C108, 0);
        func_00343538(D_001A8CFC[0] + 0x3C108, D_00259F58);
        func_00343580(D_001A8CFC[0] + 0x3C108);
        func_003435A0(D_001A8CFC[0] + 0x3C108, D_0025D0E0);
    }
    return 0;
}

extern char D_00259CE0[];
extern char D_0025D288[];

int func_002D3538(void)
{
    if (D_001A8CFC[0] != 0) {
        func_003435B0(D_001A8CFC[0] + 0x3C108, 1);
        func_00343538(D_001A8CFC[0] + 0x3C108, D_00259CE0);
        func_00343580(D_001A8CFC[0] + 0x3C108);
        func_003435A0(D_001A8CFC[0] + 0x3C108, D_0025D288);
    }
    return 0;
}


INCLUDE_ASM("asm/game", func_002D35B8);

INCLUDE_ASM("asm/game", func_002D36A0);

INCLUDE_ASM("asm/game", func_002D3D88);

INCLUDE_ASM("asm/game", func_002D3FA0);

INCLUDE_ASM("asm/game", func_002D4948);

int func_002D4E10(void)
{
    char *p = F7cb419c1_D_001F2840;

    *(int *)(p + 460) = F7cb419c1_FUN_002DE618(*(int *)(p + 460));
    return 0;
}

INCLUDE_ASM("asm/game", func_002D4E48);

INCLUDE_ASM("asm/game", func_002D4FF0);

extern int D_0025BA80;

int func_002D5038(void) {
    D_0025BA80 = 0;
    return 0;
}

INCLUDE_ASM("asm/game", func_002D5048);

INCLUDE_ASM("asm/game", func_002D50F0);

extern char *D_001A8CFC[] __attribute__((sda));
extern void *func_00341558(void *);

int func_002D5290(char *object) {
    if (D_001A8CFC[0] != 0)
        *(void **)(object + 0x34) = func_00341558(D_001A8CFC[0] + 0x3BE08);
    return 0;
}

INCLUDE_ASM("asm/game", func_002D52D8);

INCLUDE_ASM("asm/game", func_002D53F0);

extern void func_00287E80(void);
extern void func_00341540(char *, char *, char *, char *);
extern void func_00341610(char *, int);
extern char D_002615F8[], D_00261698[], D_00261750[];

int func_002D5480(void)
{
    func_00287E80();
    if (D_001A8CFC[0] != 0) {
        func_00341540(D_001A8CFC[0] + 0x3BE08, D_002615F8, D_00261698, D_00261750);
        func_00341610(D_001A8CFC[0] + 0x3BE08, 1);
    }
    return 0;
}

extern int D_00138344;
extern void func_0029CF08(int);

int func_002D54F0(void) {
    func_0029CF08(D_00138344);
    return 0;
}

extern int D_00138344;
extern void func_0029CF08(int);

int func_002D5518(void) {
    func_0029CF08(D_00138344);
    return 0;
}
