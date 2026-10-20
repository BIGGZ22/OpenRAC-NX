/* func_L18_00427B48, 148 bytes, 3 placements.  Best candidate 144 bytes; everything
   matches except the register assignment and one padding word:
   retail keeps the chain pointer in $a1 (the argument register, freed after the table
   read) and the constant 1 in $a0, this cc1 swaps the two, and retail has a bare `nop`
   after the four `addiu` constant set-ups that this build does not emit (the loop top is
   then one word later, which is why the back edge distance differs).
   Both table addresses and the 0xEC0 / 0xFE / 0xFD / 1 constants are otherwise exact.
   The body: */

typedef unsigned char u8;
typedef short s16;
typedef signed int s32;

extern char *D_L18_001FB320[];
extern char *D_L18_001B281C;

void func_L18_00427B48(s32 a0) {
    s16 *p;
    char *v1;

    if (a0 == 0xFF) {
        return;
    }
    p = (s16 *)D_L18_001FB320[a0];
    if (p == 0) {
        return;
    }
    do {
        v1 = D_L18_001B281C + ((*(unsigned short *)p & 0x7FFF) << 8);
        if (v1 != 0 && *(s16 *)(v1 + 0xAA) == 0xEC0 &&
            *(u8 *)(v1 + 0x20) != 0xFE && *(u8 *)(v1 + 0x20) != 0xFD) {
            *(u8 *)(*(char **)(v1 + 0x68) + 0x39A) = 1;
        }
    } while (*p++ >= 0);
}
