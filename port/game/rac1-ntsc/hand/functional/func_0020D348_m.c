/* functional: medium. The moby allocator at US 0x0020C4F8 (PAL 0x0020D348, which PAL keeps only as nonmatching assembly), 248 bytes,
 * named func_0020D348_m by the level code that calls it. Walks 0x100-byte records from the cursor to the end, takes the first one whose
 * state byte at +0x20 is 0xFE or more and whose 64-bit stamp at +0x38 is not after the frame stamp, initialises it with FUN_0020c5f0,
 * clears its 128-byte block (FUN_001f97e8) and points +0x78 at that block. Returns the record, or 0 after reporting through FUN_001e93b0.
 * Written from the assembly; the counter and the 0xFF tag at +0x120 are kept as retail writes them. */
#include "types.h"

#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif

extern u32 D_0015FFDC MACRO_ADDR; /* cursor: first record to try (game address) */
extern u32 D_0015FFE0 MACRO_ADDR; /* end of the record table */
extern u32 D_0015FFE8 MACRO_ADDR; /* base of the 128-byte blocks */
extern u32 D_0015FFBC MACRO_ADDR; /* count of records still to hand out, decremented when nonzero */
extern u32 D_0015F5CC MACRO_ADDR; /* frame stamp */
extern char D_001F38E0[] MACRO_ADDR; /* the report's format string */

void FUN_0020c5f0(char *rec, int type);
void FUN_001f97e8(char *dst, int value, int bytes);
void FUN_001e93b0(char *fmt, u32 value);

char *func_0020D348_m(int type) {
    u32 p = D_0015FFDC;
    u32 end = D_0015FFE0;
    u32 stamp_now;
    s32 index;
    char *block;

    while (p < end) {
        u8 state = *(u8 *)(p + 0x20);
        if (state >= 0xFE) {
            u64 stamp = *(u64 *)(p + 0x38);
            stamp_now = D_0015F5CC;
            if (!((u64)stamp_now < stamp)) {
                if (state == 0xFF) {
                    *(u8 *)(p + 0x120) = 0xFF;
                }
                FUN_0020c5f0((char *)p, type);
                index = (s32)(p - D_0015FFDC);
                index >>= 8;
                index <<= 7;
                block = (char *)(D_0015FFE8 + (u32)index);
                FUN_001f97e8(block, 0, 0x80);
                *(u32 *)(p + 0x78) = (u32)block;
                if (D_0015FFBC != 0) {
                    D_0015FFBC -= 1;
                }
                return (char *)p;
            }
        }
        p += 0x100;
    }
    FUN_001e93b0(D_001F38E0, D_0015F5CC);
    return 0;
}
