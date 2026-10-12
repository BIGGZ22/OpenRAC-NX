#include "include_asm.h"

extern int func_0033E348(int);

extern int func_00348128(unsigned char *);

/* Qwen draft (branch qwen/autonomy, 4419d477), checked by tools/check.py */
void func_0033AF48(int);

extern void func_0033AB50_1(void *);

extern void func_L00_0042BFF8(void *);

extern void func_L00_0042C290(void *);

extern void func_L00_0042BAD0(void *);

int func_0033E348(int owner)
{
    return func_00348128((unsigned char *)(owner + 0x2b0));
}

INCLUDE_ASM("asm/game", func_0033E368);

INCLUDE_ASM("asm/game", func_0033E3A0);

INCLUDE_ASM("asm/game", func_0033E428);

INCLUDE_ASM("asm/game", func_0033E480);

INCLUDE_ASM("asm/game", func_0033E4B8);

INCLUDE_ASM("asm/game", func_0033E4C0);

INCLUDE_ASM("asm/game", func_0033E5C0);

void func_0033E620(int param_1) {
  func_0033AF48(param_1 + 8);
}

/* Calls twelve sub-object handlers in sequence, each with the object base plus a fixed byte offset (8, 684, 744, 816, 888, 960, 1040, 1112, 1184, 1256, 1328, 1400), then returns the object pointer. */
char *func_0033E640(char *object)
{
    func_0033AB50_1(object + 8);
    func_L00_0042BFF8(object + 684);
    func_L00_0042C290(object + 744);
    func_L00_0042C290(object + 816);
    func_L00_0042C290(object + 888);
    func_L00_0042BAD0(object + 960);
    func_L00_0042C290(object + 1040);
    func_L00_0042C290(object + 1112);
    func_L00_0042C290(object + 1184);
    func_L00_0042C290(object + 1256);
    func_L00_0042C290(object + 1328);
    func_L00_0042C290(object + 1400);
    return object;
}

INCLUDE_ASM("asm/game", func_0033E6C8);

INCLUDE_ASM("asm/game", func_0033E6D0);

INCLUDE_ASM("asm/game", func_0033EA60);
