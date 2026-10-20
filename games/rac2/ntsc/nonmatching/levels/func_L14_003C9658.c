/* func_L14_003C9658, 136 bytes, 1 placement.  Nearest: 132 bytes, four words.
   Guards on the object's 0x95 byte being 1 and its 0x20 byte not being 7, then
   func_420C68(*(s1 + 0x220), a0 + 0x10, a1) with s1 = *(a0 + 0x68); on a non-zero result it
   calls func_321F58(a0, 7, -1), writes 0x3C to s1 + 0x37C and copies the 16 bytes at a1 to
   s1 + 0x340 with lq/sq.
   Measured gaps: (1) cc1 folds the +0x340 into the sq (sq $v0,0x340($s1)) where retail
   materialises `addiu $a0,$s1,0x340` and stores at offset 0 - the wall the wave-6 note
   records for the TI128 copies; (2) the func_420C68 argument setup lands in a different
   order (retail a0, a1, a2; here a1, a2, then a0 after the jal). */

#include "include_asm.h"
#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_321F58();
extern int func_420C68();

void func_L14_003C9658(char *a0, char *a1)
{
    char *s2 = a1;
    char *s1;

    if (*(u8 *)(a0 + 0x95) != 1)
        return;
    if (*(u8 *)(a0 + 0x20) == 7)
        return;
    s1 = *(char **)(a0 + 0x68);
    if (func_420C68(*(s32 *)(s1 + 0x220), a0 + 0x10, s2) == 0)
        return;
    func_321F58(a0, 7, -1);
    *(s32 *)(s1 + 0x37C) = 0x3C;
    *(TI128 *)(s1 + 0x340) = *(TI128 *)s2;
}
