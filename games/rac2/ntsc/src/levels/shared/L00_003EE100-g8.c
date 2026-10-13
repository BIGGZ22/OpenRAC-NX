#include "include_asm.h"

/* A body whose retail bytes address its global through $gp in ordinary flow: the assembler
   only selects that form when the back end emits the .extern size directive, which the
   pinned profile does at -G8 (see docs/COMPILER-NOTES.md). */
extern unsigned char *func_0029EFD0(int size);

extern unsigned char *D_L00_001B0A20 __attribute__((sda));

unsigned char *func_L00_003F8DC8(void)
{
    unsigned char *p = func_0029EFD0(0xD04);

    if (p != 0) {
        unsigned int v = 0xFF;

        D_L00_001B0A20 = p;
        *(unsigned short *)(p + 0x32) = v;
        p[0x20] = 1;
        p[0x30] = v;
    }
    return p;
}
