#include "include_asm.h"

typedef unsigned char u8;
typedef int s32;

/* func_L02_003FC1B0 -- 152 bytes, level 2.  The body is fully understood (a switch on
 * the id in a1 that increments one of four counters in the sub-object at a0+0x68 and
 * returns the new value); the wall is the decision tree this cc1 builds.
 *
 * retail:
 *   0x003FC1B0  li v0,0xA44 ; beq a1,v0,+0x20 ; lw v1,0x68(a0) [delay]
 *   0x003FC1BC  slti v0,a1,0xA45 ; beqz v0,+0x28 ; li v0,0x9CA [delay]
 *   0x003FC1C8  beql a1,v0,+0x60 ; lw v0,0x50(v1) [delay]
 *   0x003FC1D0  jr ra (the 0xA44 arm, returns the constant already in v0)
 *   0x003FC1D8  li v0,0xC3B ; beq a1,v0,+0x78 ; slti v0,a1,0xC3C [delay]
 *   0x003FC1E4  beqz v0,+0x4C ; li v0,0xB22
 *   0x003FC1EC  beql a1,v0,+0x6C ; lw v0,0x54(v1)
 *   0x003FC1F4  jr ra ; 0x003FC1FC li v0,0xC3E ; beql a1,v0,+0x88 ; lw v0,0x5C(v1)
 *   0x003FC208  jr ra ; arms: 0x60 ++0x50, 0x6C ++0x54, 0x78 ++0x58, 0x88 ++0x5C
 *
 * measured: the first test is the LOWEST case (0xA44) with a 0xA45 pivot, and the arm
 * order is 0xA44, 0x9CA, 0xC3B, 0xB22, 0xC3E - not the sorted, balanced tree this cc1
 * builds for the same five cases (it opens on 0xB22 and lays the arms out of line).
 * Eight source shapes were measured: switch with the arms in retail's order, switch
 * with `break` and a result variable, switch with a bare `return 0xA44` tail, and the
 * if/else chains written straight from the retail tree (both with early returns and
 * with one result variable).  Best candidate 156 bytes against 152, first difference
 * already at +0x4 (the 0xA44 arm placed out of line), so the remaining gap is the
 * compiler's block layout, not the source.  -fno-schedule-insns2 and -O1 were not
 * tried (the file's profile is the repo default).
 */

s32 func_L02_003FC1B0(u8 *a0, s32 a1) {
    s32 *v1 = *(s32 **)(a0 + 0x68);

    if (a1 == 0xA44)
        return 0xA44;
    if (a1 < 0xA45) {
        if (a1 == 0x9CA)
            return ++v1[0x14];
        return 0x9CA;
    }
    if (a1 == 0xC3B)
        return ++v1[0x16];
    if (a1 < 0xC3C) {
        if (a1 == 0xB22)
            return ++v1[0x15];
        return 0xB22;
    }
    if (a1 == 0xC3E)
        return ++v1[0x17];
    return 0xC3E;
}
