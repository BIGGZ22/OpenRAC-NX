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

extern void func_0029C450(void);

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

extern int F750245c6_D_001A7340 __attribute__((sda));

extern void F750245c6_FUN_0028FC48(int a, int b, int c, int d, char *e, int f);

long long func_002CD8C0(void)
{
    func_0027C540(0LL);
    func_0029C450();
    func_0027C660();
    return 0LL;
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void func_00341550(void *, int);
int func_002CD8F0(void *object) { char *root=D_001A8CFC[0]; if (root != 0) { func_00341550(root+0x3BE08,1); *(void **)((char *)object+0x34)=func_00341558(D_001A8CFC[0]+0x3BE08); } return 0; }

INCLUDE_ASM("asm/game", func_002CD950);

INCLUDE_ASM("asm/game", func_002CDA20);

INCLUDE_ASM("asm/game", func_002CDAB0);

INCLUDE_ASM("asm/game", func_002CDB30);

INCLUDE_ASM("asm/game", func_002CDD88);

extern int D_001AB9B0 __attribute__((sda));

int func_002CE5B8(void) {
    D_001AB9B0 = 1;
    return 0;
}

int func_002CE5C8(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002CE5D0);

INCLUDE_ASM("asm/game", func_002CEBD0);

void func_002CF0A0(int a0, char *a1)
{
    int h, k, p, m;
    k = a0 / 2 + 5;
    h = F750245c6_D_001A7340 / 2;
    m = h - k;
    p = h + k;
    F750245c6_FUN_0028FC48(m - 2, 312, p + 4, 314, a1, 0);
    F750245c6_FUN_0028FC48(m - 2, 333, p + 4, 335, a1, 0);
    F750245c6_FUN_0028FC48(m - 2, 313, m, 334, a1, 0);
    F750245c6_FUN_0028FC48(p + 2, 313, p + 4, 334, a1, 0);
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void *func_00341558(void *);

int func_002CF188(char *object) {
    if (D_001A8CFC[0] != 0)
        *(void **)(object + 0x34) = func_00341558(D_001A8CFC[0] + 0x3BE08);
    return 0;
}

INCLUDE_ASM("asm/game", func_002CF1D0);

INCLUDE_ASM("asm/game", func_002CF2B8);

INCLUDE_ASM("asm/game", func_002CF340);

INCLUDE_ASM("asm/game", func_002CF5B0);

INCLUDE_ASM("asm/game", func_002CF7F0);

int func_002CFBE0(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002CFBE8);

INCLUDE_ASM("asm/game", func_002CFDA8);

int func_002D01C8(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002D01D0);

INCLUDE_ASM("asm/game", func_002D0568);

INCLUDE_ASM("asm/game", func_002D08C8);

INCLUDE_ASM("asm/game", func_002D0930);

INCLUDE_ASM("asm/game", func_002D0B98);

INCLUDE_ASM("asm/game", func_002D0F00);

INCLUDE_ASM("asm/game", func_002D0FA0);

INCLUDE_ASM("asm/game", func_002D1228);

INCLUDE_ASM("asm/game", func_002D1590);

INCLUDE_ASM("asm/game", func_002D16C8);

INCLUDE_ASM("asm/game", func_002D1970);

INCLUDE_ASM("asm/game", func_002D1CB0);

INCLUDE_ASM("asm/game", func_002D1D20);

INCLUDE_ASM("asm/game", func_002D1E70);

int func_002D2010(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002D2018);

INCLUDE_ASM("asm/game", func_002D2188);

extern char *D_001A8CFC[] __attribute__((sda));
extern int D_0025CADC;

int func_002D23D8(void) {
    if (D_001A8CFC[0] != 0) D_0025CADC = -300;
    return 0;
}

INCLUDE_ASM("asm/game", func_002D23F8);

INCLUDE_ASM("asm/game", func_002D2538);

int func_002D2830(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002D2838);

INCLUDE_ASM("asm/game", func_002D2918);

extern int D_00138344;
extern void func_0029C640(int);

int func_002D2BD8(void) {
    func_0029C640(D_00138344);
    return 0;
}

extern void func_0029C610(void);

int func_002D2C00(void) {
    func_0027C540(0);
    func_0029C610();
    func_0027C660();
    return 0;
}

typedef struct {
    char reserved00[0x14];
    char *source;
    int value;
    char reserved1c[0x118];
    int fallback;
} CallbackState;
extern CallbackState D_001F2840;
extern int D_00138344;
extern void func_0029C6B0(int);

int func_002D2C30(void) {
    int result = 0;
    int flags = D_00138344;
    if (flags & 0x10) {
        int value = *(int *)(D_001F2840.source + 0xE0);
        if (value != 0)
            D_001F2840.value = value;
        else if (D_001F2840.fallback == 0) {
            value = -1;
            goto return_value;
        }
        value = 0;
return_value:
        return value;
    }
    if (flags & 0x900)
        result = 1;
    else
        func_0029C6B0(flags);
    return result;
}
