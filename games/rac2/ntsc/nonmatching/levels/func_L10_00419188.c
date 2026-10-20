/* 0 of 1 places match - last line: 10_hrugis_cloud: 88 bytes, expected 96.
   ROW IS EIGHT BYTES TOO LONG. The candidate walks the same list as func_L10_00417440 with the
   store value 2; retail's real body is 88 bytes (0x419188..0x4191E0: `lbu 0x21/$a0`, the
   lui/addiu table at 0x1FB1E0, `beqz` to the shared epilogue, then the lhu/lh pair loop).
   The row claims 0x419188+0x60 and the extra eight bytes are an UNREACHABLE second epilogue
   `jr $ra; nop` at 0x4191E0 - both the `beqz` and the loop exit reach 0x4191D8 - and the next
   catalogue row (func_L10_004191E8) starts exactly at 0x4191E8. Maintainer data: re-cut the
   row to 0x58.

   The candidate below is the 88-byte body, and it is now byte for byte the retail's 88 bytes:
   the table entry is loaded into $v0 and copied with `daddu $a0,$v0,$zero` in the `beqz`
   delay slot, the D_001B251C address is rebuilt inside the loop (the declaration carries the
   sda attribute, which leaves the address to the assembler), the do-while's post-increment
   sits in the back branch's delay slot, and the store keeps the base in $v1 with the
   0x7FFF mask shifted by 8. The four bytes the earlier candidate was short came from
   declaring D_001B251C as sda (cc1 then emits the bare symbol and the assembler rebuilds
   the address in the loop) and from reading the table entry as a u32 (the copy of the
   pointer is then a 64-bit `daddu`).

   Only the eight extra bytes of the row are left, and they are not code this C can reach.
   Below: the whole candidate (declarations as used in src/levels/10_hrugis_cloud/L10_003170D0.c). */

#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;

extern u32 D_L10_001FB1E0[];
extern u8 *D_001B251C __attribute__((sda));

void func_L10_00419188(u8 *arg0)
{
    u32 s0 = D_L10_001FB1E0[arg0[0x21]];
    s16 *p;
    u8 *q;

    if (s0 == 0) {
        return;
    }
    p = (s16 *)s0;
    do {
        q = D_001B251C + (((*(u16 *)p & 0x7FFF) << 8));
        *(q + 0x20) = 2;
    } while (*p++ >= 0);
}
