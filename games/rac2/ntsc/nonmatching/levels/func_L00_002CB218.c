/* 0 of 27 places match - last line: 00_aranos_tutorial: 192 bytes, expected 188
   Candidate size exact, and two of the three remaining differences are the walls below:
   (1) the +0x24F8 word is `lwu` in every place - this cc1 only emits `lwu` for a 64-bit
       variable holding an unsigned 32-bit load (measured: `u64 w = *(u32 *)p;`); the plain
       `*(u32 *)p == 0` and every other spelling give `lw`;
   (2) retail keeps the 0x00138180 base in one register and uses two plain offsets, this
       cc1 folds each access into its own %hi/%lo pair (address CSE) or, with a pointer
       variable, materialises the base above the first test instead of at its first use;
   192 against 188 because the store before func_2FC008 keeps its own instruction here.
   The 0x0018C318 base is loaded and then walked to 0x00189E20 with `addiu -0x24F8`, which
   only comes out when one symbol carries both addresses. */

#include "common.h"

extern char D_0018C318[];
extern char D_00138180[];
extern int D_001A8FB4;
extern void func_2EDD78(void *);
extern void func_2FC008(void);

void func_L00_002CB218(void)
{
    char *b;
    char *o;
    u64 w;

    func_2EDD78(D_0018C318);
    func_2EDD78(D_0018C318 + 2);
    b = D_0018C318 - 0x24F8;
    o = D_00138180;
    if (0x37 == *(int *)(b + 0x2294))
        return;
    if (*(u8 *)(b + 0x22BC) != 0)
        return;
    if (D_001A8FB4 != 0)
        return;
    if (*(int *)(o + 0x1A4) & 0x10)
        *(short *)(b + 0x24F8) = 20;
    w = *(u32 *)(b + 0x24F8);
    if (w == 0)
        return;
    if (*(int *)(o + 0x1B8) & 0x10) {
        if (*(short *)(b + 0x24FA) == 0) {
            *(short *)(b + 0x24FA) = 20;
            return;
        }
        *(int *)(b + 0x2538) = 1;
        func_2FC008();
    }
}
