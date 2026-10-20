/* func_L14_003E50B8, 212 bytes, 1 placement.  Nearest: 208 bytes, one register set.
   Walks the s16 ring at *(s16 *)(a1 + 0x16) forward (mod 8, a1 = *(a0 + 0x68)) until the entry
   at a1 + 2 * index stops exceeding the limit at a1 + 0x14, stores the wrapped index back, and
   then, on an odd index, calls func_321F58(a0, 2, -1) unless the object's 0x20 byte is already
   2 (even index: the same with 1).
   The shape is reproduced exactly; only the temporaries differ: retail keeps the base in $a1,
   the limit in $a2, the index in $a0 and the parameter copy in $a3 (with $v0/$v1 for the ring
   loads), this cc1 assigns $a2/$a3/$a0 plus $t0 for the parameter.  Five spellings (s16 vs
   s32 index, the (s16) cast before or after the increment, an explicit parameter copy or not,
   the index loaded before the limit) all land on the same 208 bytes. */

#include "include_asm.h"
#include "common.h"

extern void func_321F58();

void func_L14_003E50B8(char *a0)
{
    char *a3 = a0;
    char *a1 = *(char **)(a3 + 0x68);
    s16 lim = *(s16 *)(a1 + 0x14);
    s32 i = *(s16 *)(a1 + 0x16);

    if (lim < *(s16 *)(a1 + (s16)i * 2)) {
        do {
            i = ((s16)i + 1) % 8;
            *(s16 *)(a1 + 0x16) = i;
        } while (lim < *(s16 *)(a1 + (s16)i * 2));
    }
    if (*(u16 *)(a1 + 0x16) & 1) {
        if (*(u8 *)(a3 + 0x20) != 2)
            func_321F58(a3, 2, -1);
    } else {
        if (*(u8 *)(a3 + 0x20) != 1)
            func_321F58(a3, 1, -1);
    }
}
