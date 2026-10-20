/* 0 of 3 places match - last line: 02_maktar_nebula: 128 bytes, expected 124.
   Same wall as the note already in src/levels/shared/L02_003C43F0.c (see
   work/wave7-20261013/w04/drafts, 116 bytes): the loop, the two branch-likely
   tests and the duplicated `lh` delay-slot loads are retail's.  This candidate
   (written independently before the note was found) keeps the kind byte in $v1
   against retail's $v0 and its comparison constants one register lower
   ($a2/$a3 against $a3/$t0), plus a `nop` the retail loop does not have.
   The pointer base is materialised absolutely (nosda); the byte pool is a
   one-instruction `lw $a2,%lo($gp)` load, so the unit needs -G8. */

#include "include_asm.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef short s16;
typedef unsigned char u8;

extern u8 *D_L02_001FC060[] __attribute__((nosda));
extern u8 D_L02_001B35A0[] __attribute__((sda));

/* walks the halfword list of the element selected by the byte at +0x21 and tags
   the 0xA4E records that carry a negative flag at +0x20 */
void func_L02_003CE9A8(u8 *object)
{
    u8 *entry;
    u8 *p;
    u8 *pool = D_L02_001B35A0;

    if (object[0x21] == 0xFF)
        return;
    entry = D_L02_001FC060[object[0x21]];
    if (entry == 0)
        return;

    p = entry;
    do {
        u16 word = *(u16 *)p;
        u8 *q = pool + ((word & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0 && *(s16 *)(q + 0xAA) == 0xA4E)
            *(u16 *)(*(s32 *)(q + 0x68) + 0x316) = 1;
        p += 2;
    } while (*(s16 *)p >= 0);
}
