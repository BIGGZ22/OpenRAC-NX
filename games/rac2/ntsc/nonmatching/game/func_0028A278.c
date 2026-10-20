#include "include_asm.h"

extern int D_001A79F8;
extern int D_001B1948;
extern int D_001B194C;
extern int D_001B1950;
extern int D_001B1954;
extern int D_001B1958;
extern int D_001B1928[];
extern void func_0029CBA8();

/* Near miss for func_0028A278 (boot 0x0028A278, 27 levels, 128 bytes).
 *
 * The body is recovered: the value in D_001A79F8 is copied to D_001B194C and D_001B1948,
 * D_001B1950 is cleared, func_0029CBA8 is called with &D_001B1948, D_001B1958 is cleared,
 * D_001B1954 gets -1 and one 16-byte entry of the table at D_001B1928 is zeroed.
 *
 * Every instruction and every address is right; the residue is how the addresses of the
 * globals are materialised: retail puts each one-off address in `$at` and offsets from it
 * (`lui $at,0x1B; sw $v0,0x194C($at)`), this cc1 keeps them in ordinary registers and
 * offsets by zero.  The same count of words, one different word per global.
 */
void func_0028A278(void) {
    int v = D_001A79F8;
    D_001B194C = v;
    D_001B1948 = v;
    D_001B1950 = 0;
    func_0029CBA8((int)&D_001B1948);
    D_001B1958 = 0;
    D_001B1954 = -1;
    {
        int *p = D_001B1928;
        int i = 1;
        do {
            *(unsigned short *)((char *)p + 0xE) = 0;
            *p = 0;
            p += 4;
            i--;
        } while (i >= 0);
    }
}
