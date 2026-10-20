#include "include_asm.h"

extern void func_002FC560(int, long long);

extern void func_L00_003EA8C8(char *a, unsigned int b, unsigned int c, float d);

/* the level's table, named twice: the address is materialised again for each
 * call instead of being kept in a register across them */
extern int D_L00_001B2BA0[];
extern int D_L00_001B2BA0_1[];

/* Puts the object in its 0x3C-tick state: a sound, two tinted texts and a
 * last sound, but only when the object is not already in it. */
void func_L00_003EA7D0(char *p)
{
    if (*(unsigned char *)(p + 0x31) == 0)
        return;

    func_002FC560(0x47, 0x53001LL);
    func_L00_003EA8C8(p, (unsigned int)D_L00_001B2BA0, 0x7F10307F, 0.25f);
    func_L00_003EA8C8(p, (unsigned int)D_L00_001B2BA0_1, 0x3F08183F, 0.5f);
    func_002FC560(0x47, 0x5360BLL);
}
