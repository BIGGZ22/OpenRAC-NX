/* 0 of 1 places match - last line: 18_damosel: 244 bytes, expected 196
   Same walk family as func_L20_00444138 (recipe from the matched L01_003CAFE0-g8.c and
   L04_00422A00-g8.c): the pre-loop `if (a0[0x21] == 0xFF) return;`, `p = table[a0[0x21]]`,
   then a loop whose test re-reads the key every pass and whose id arm is
   `(u16)(id - 0x920) < 2` plus a `(s16)id == id` re-check, with a nested
   `p == table[a0[0x21]]` continue test in the same condition block. Body shape confirmed:
   `base = D_L18_001B281C; e = base + ((*p & 0x7FFF) << 8); if (e == 0) break;
   func_L18_00432AD0(e); p++;` - the `bnel`/`bgezl`/`beql` block that retail builds for the
   test (`lh -0x2($s0)` then `bgezl` with `lhu 0x0($s0)` in the slot) is what a
   `*(s16 *)(p - 1) >= 0 || p == table[i]` tail compiles to.
   The wall is allocation, not shape: retail keeps only the table's %hi constant in `$s2`
   (`daddu $s2,$v1,$zero` after `lui $v1,0x20`) and re-materialises `addiu $v1,$s2,-0x4CE0`
   per site, in a 0x20 frame saving s0-s2; this cc1 keeps the *whole* base live in `$s4`
   (plus a `$s3`), which pushes the frame to 0x30 and shifts every word.
   Measured: the same C with the (s16)<0 && p!=table[i] test moved into the body as a `break`
   is identical (244); a single `$gp`-declared bound and an inline `while` give the same. */
#include "common.h"

extern char *D_L18_001B281C __attribute__((nosda));

extern int D_L18_001B2838 __attribute__((sda));

extern u32 D_L18_001FB320[];

extern void func_L18_00432AD0(char *);

/* Walks the 0x100-stride object list hanging off the record's 0x21 byte, skipping the
   0x920-0x921 class the sub-record at state+0x9C belongs to, and hands each surviving
   object to func_L18_00432AD0. */
void func_L18_00432D60(char *arg0)
{
    char *base;
    char *e;
    u16 *p;

    if (*(u8 *)(arg0 + 0x21) == 0xFF)
        return;
    p = (u16 *)D_L18_001FB320[*(u8 *)(arg0 + 0x21)];
    while (*(u8 *)(arg0 + 0x21) != 0xFF &&
           *(u8 *)(arg0 + 0x21) <= D_L18_001B2838 && p != 0 &&
           (*(s16 *)(p - 1) >= 0 ||
            p == (u16 *)D_L18_001FB320[*(u8 *)(arg0 + 0x21)])) {
        base = D_L18_001B281C;
        e = base + ((*p & 0x7FFF) << 8);
        if (e == 0)
            break;
        func_L18_00432AD0(e);
        p++;
    }
}
