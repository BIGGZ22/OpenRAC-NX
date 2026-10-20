/* 0 of 2 places match - last line: 03_endako: instruction at +0x14 differs.
   Every instruction matches but the register of the fetched state block: this
   cc1 keeps it in $v1 and moves it to $a1 for the +0x80 argument, where retail
   loads it straight into $a1 once the incoming pointer has been moved to $a0.
   Naming the incoming pointer in a local first did not move it.  Register
   allocation, not a source shape. */

#include "include_asm.h"

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;

extern u8 *D_L03_001B2A1C __attribute__((sda));
extern void func_L02_0032EDD8(u8 *, u8 *, s32, s32 *);

/* reads the element's tracked value through a sentinel-initialised slot */
s32 func_L03_00419C58(u32 a0, u8 *a1) {
    u8 *q = *(u8 **)((u8 *)D_L03_001B2A1C + (a0 << 8) + 0x68);
    s32 out = -1;

    func_L02_0032EDD8(a1, q + 0x80, *(s32 *)(q + 0x480), &out);
    return out;
}
