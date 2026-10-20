/* func_L00_002A6708, 240 bytes, 27 placements.  ABANDON - loop-shape wall (register pair
   and loop rotation).
   Walks the nine slots of two resident arrays (destination at +0x242C, source at +0x2450)
   and retires the record each slot names, clearing the slot to -1 either way: only a record
   whose +0x88 word is the live resident pointer (for a slot with id 0) or the slot's own id,
   and whose +0x74 flag byte is set, reaches func_35F0A8. Retail advances three values per
   iteration: the destination pointer (s0, +4), an independent ascending byte offset
   (s3, +4) for `0x2450 + offset`, and a down-counting trip counter (s2).
   Closest candidate below is 228 bytes; last line of tools/check.py was
   "00_aranos_tutorial: instruction at +0x50 differs". Still open: this cc1 rotates the
   three increments to the top of the body where retail leaves them at the bottom, gives
   s1/s2 to the constant -1 and the counter the other way round than retail, and emits
   `addu $v0,$s7,$s3` where retail emits `addu $v0,$s3,$s7`.
   Unblocking: an idiom that keeps the increments at the loop end, or a spelling that makes
   the -1 constant take $s1 and the counter $s2. Round variants tried: for-clause orders
   i++/dst++/offset+=4 in both orders, i<=8, declared offset first and second, both arrays as
   pointers, do-while with the increments written in the body (closest), and the source read
   as base+0x2450+i*4, src+offset and src[i]. */

#include "include_asm.h"

typedef unsigned char u8;

extern char D_00189E20_2[];
extern char D_00188860[];
extern void func_35F0A8(int entry);

void func_L00_002A6708(void)
{
    char *base = D_00189E20_2;
    int *dst = (int *)(base + 0x242C);
    char *src = base + 0x2450;
    int i = 0;
    int offset = 0;

    do {
        int id = *(int *)(src + offset);
        int cur = *dst;
        char *entry;

        if (id == 0) {
            if (cur == -1)
                goto clear;
            entry = D_00188860 + cur * 0x70;
            if (*(int *)(entry + 0x88) != *(int *)(base + 0x2290))
                goto clear;
        } else {
            if (cur == -1)
                goto clear;
            entry = D_00188860 + cur * 0x70;
            if (*(int *)(entry + 0x88) != id)
                goto clear;
        }
        if (*(u8 *)(entry + 0x74) == 0)
            goto clear;
        func_35F0A8(cur);
    clear:
        *dst = -1;
        dst++;
        offset += 4;
        i++;
    } while (i < 9);
}
