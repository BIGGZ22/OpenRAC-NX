#include "include_asm.h"

extern int D_L11_001B359C __attribute__((sda));

extern void func_32C700();

INCLUDE_ASM("asm/levels", func_L11_00432628);

INCLUDE_ASM("asm/levels", func_L11_00434580);

INCLUDE_ASM("asm/levels", func_L11_004349F8);

INCLUDE_ASM("asm/levels", func_L11_00434AA8);

INCLUDE_ASM("asm/levels", func_L11_00434AF8);

INCLUDE_ASM("asm/levels", func_L11_00434D00);

INCLUDE_ASM("asm/levels", func_L11_00434F48);

INCLUDE_ASM("asm/levels", func_L11_00435228);

INCLUDE_ASM("asm/levels", func_L11_00435600);

INCLUDE_ASM("asm/levels", func_L11_00436350);

INCLUDE_ASM("asm/levels", func_L11_00436768);

/* Re-arms the measured stage guard, then clears it. */
INCLUDE_ASM("asm/levels", func_L11_00436850);

INCLUDE_ASM("asm/levels", func_L11_00436910);

INCLUDE_ASM("asm/levels", func_L11_00436A78);

INCLUDE_ASM("asm/levels", func_L11_00436B78);

INCLUDE_ASM("asm/levels", func_L11_00436D08);

INCLUDE_ASM("asm/levels", func_L11_00436F80);

INCLUDE_ASM("asm/levels", func_L11_004375B8);

INCLUDE_ASM("asm/levels", func_L11_00437CF0);

/* Walks the ten measured indices, clearing each record selected through 0x330. */
void func_L11_00437DC0(char *p, int a1, int a2, int a3)
{
    int i;

    for (i = 0; i < 10; i++) {
        int offset = i * 4;
        int index = *(int *)(*(char **)(p + 0x68) + offset + 0x330);
        char *record = (char *)(D_L11_001B359C + (index << 8));

        if (*(unsigned char *)(record + 0x43) != a1)
            func_32C700(record, a1, a2, a3);
    }
}

/* Invalidates the measured slot at 0x43 of the 256-byte record indexed by 0x358. */
void func_L11_00437E68(char *p, int a1)
{
    int record = (*(int *)(*(char **)(p + 0x68) + 0x358) << 8) + D_L11_001B359C;

    if (*(unsigned char *)(record + 0x43) != a1)
        func_32C700((char *)record, a1);
}

INCLUDE_ASM("asm/levels", func_L11_00437EA8);

INCLUDE_ASM("asm/levels", func_L11_00437FD0);

INCLUDE_ASM("asm/levels", func_L11_00438248);

INCLUDE_ASM("asm/levels", func_L11_00438320);

INCLUDE_ASM("asm/levels", func_L11_00438460);

INCLUDE_ASM("asm/levels", func_L11_004386B8);

INCLUDE_ASM("asm/levels", func_L11_00438880);

/* Clears the two 16-bit flags at 0x374/0x376 of the object held at 0x68. */
void func_L11_00438CD8(char *p)
{
    char *q = *(char **)(p + 0x68);
    *(short *)(q + 0x374) = 0;
    *(short *)(q + 0x376) = 0;
}

INCLUDE_ASM("asm/levels", func_L11_00438CE8);

INCLUDE_ASM("asm/levels", func_L11_00438F18);

INCLUDE_ASM("asm/levels", func_L11_004390D0);

INCLUDE_ASM("asm/levels", func_L11_004393D8);

INCLUDE_ASM("asm/levels", func_L11_004395C8);

INCLUDE_ASM("asm/levels", func_L11_00439640);

INCLUDE_ASM("asm/levels", func_L11_00439950);

INCLUDE_ASM("asm/levels", func_L11_00439BF0);

/* Picks the measured 3/4 code from the counter at 0x364 of the object at 0x68. */
int func_L11_00439D60(char *p)
{
        if (*(int *)(*(char **)(p + 0x68) + 0x364))
        return 3;
    return 4;
}

INCLUDE_ASM("asm/levels", func_L11_00439DD0);

INCLUDE_ASM("asm/levels", func_L11_0043A3F8);

extern void func_32E480(char *p, char *q, int a, float b, float c, float d, int e, int f, int g);

/* Bumps the 16-bit counter at 0x96 while it is under the measured ceiling, then re-arms. */
void func_L11_0043A620(char *p)
{
    unsigned short count = *(unsigned short *)(p + 0x96);

    if (count <= 0xFFFE)
        *(unsigned short *)(p + 0x96) = count + 1;
    func_32E480(p, p + 0x10, 0x4000000, 1.2f, 1.0f, 1.0f, 0, 1, 0);
}

INCLUDE_ASM("asm/levels", func_L11_0043A680);

/* True when the byte at 0x20 is above the measured small-id range. */
int func_L11_0043A800(char *p)
{
    return *(unsigned char *)(p + 0x20) >= 4;
}

INCLUDE_ASM("asm/levels", func_L11_0043A810);

INCLUDE_ASM("asm/levels", func_L11_0043A9D8);

INCLUDE_ASM("asm/levels", func_L11_0043AB28);

INCLUDE_ASM("asm/levels", func_L11_0043ACB0);

/* Maps the measured selector onto its small index. */
INCLUDE_ASM("asm/levels", func_L11_0043E090);

/* Maps the measured selector onto its large index. */
INCLUDE_ASM("asm/levels", func_L11_0043E108);
