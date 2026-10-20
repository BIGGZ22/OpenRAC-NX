/* func_L16_0040D188 (208 bytes, 1 place, level 16) -- measured 0 of 1: SIZE IS
   RIGHT (208), the residue is one instruction order plus the allocation.

   MEASURED WITH `-O2 -G8 -ffunction-sections`: the unit needs GPREL16 for the
   `lw $v0,0x3B48($gp)` bound, so whoever promotes this must put the function in a
   file named `...-g8.c` (or give it a cflags line); L16_00403900.c, which holds it
   today, is the default -G0 unit. The bound is `extern s32 D_... __attribute__
   ((sda))`; the two other globals stay absolute (`nosda`).

   Structure: the id-list walk of src/levels/01_oozla/L01_003CAFE0-g8.c, with the
   outer guards sharing one test block at the bottom:
     init   if (arg0[0x21] == 0xFF) return; p = (u16 *)D_001FB660[arg0[0x21]];
     body   e = D_001B2B1C + ((*p & 0x7FFF) << 8);
            if (e == 0) return;
            c = *(s16 *)(e + 0xAA);
            if (c == 0xF35) *(u32 *)(*(u8 **)(e + 0x68) + 0x244) = 1;
            else if (c == 0xEA3) *(u32 *)(*(u8 **)(e + 0x68) + 0x24C) = 1;
            p++;
     test   if (arg0[0x21] == 0xFF) return;
            if (D_<gp> < arg0[0x21]) return;
            if (p == 0) return;
            if (*(s16 *)(p - 1) >= 0) goto body;                 /* list continues */
            if (p == (u16 *)D_001FB660[arg0[0x21]]) goto body;   /* head reloaded */
            return;

   THREE shapes reach exactly 208 bytes (out of 14 measured): a `for (;;)` with the
   body first and those guards at the bottom, that same form with the class value
   declared first or last, and a body-first form whose inner `do { } while (*p++ >=
   0)` is separate. The first is kept below. Everything else costs 4-24 bytes more:
   the single pre-tested `while` carrying the whole compound condition (232), the
   same with `p == head` first (228), the do/while carrying every guard (180), the
   old `p[j]`-style walk (224).

   What still differs is not the shape. Retail opens with `daddu $a2,$a0,$zero`
   (the parameter moved out of `$a0`, which the body then uses for the class value)
   and materialises 0xF35 / 0xEA3 / 0xFF / 1 at their points of use in `$v0`; this
   cc1 keeps the parameter in `$a0` and hoists all four constants into `$t0`-`$t3`
   before the loop, so the first word differs and the schedule follows it.
   Spelling the parameter copy (`u8 *st = arg0;`, used from the loop on), reading
   it through a pointer local, and several declaration orders were measured: still
   208 with the same first-word difference. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef int s32;
typedef short s16;

extern u32 D_L16_001FB660[];
extern u8 *D_L16_001B2B1C __attribute__((nosda));
extern s32 D_L16_001B3B48 __attribute__((sda));

void func_L16_0040D188(u8 *arg0) {
    s32 c;
    u16 *p;
    u8 *base;
    u8 *e;

    if (arg0[0x21] == 0xFF)
        return;
    p = (u16 *)D_L16_001FB660[arg0[0x21]];
    for (;;) {
        base = D_L16_001B2B1C;
        e = base + ((*p & 0x7FFF) << 8);
        if (e == 0)
            return;
        c = *(s16 *)(e + 0xAA);
        if (c == 0xF35)
            *(u32 *)(*(u8 **)(e + 0x68) + 0x244) = 1;
        else if (c == 0xEA3)
            *(u32 *)(*(u8 **)(e + 0x68) + 0x24C) = 1;
        p++;
        if (arg0[0x21] == 0xFF)
            return;
        if (D_L16_001B3B48 < arg0[0x21])
            return;
        if (p == 0)
            return;
        if (*(s16 *)(p - 1) < 0 && p != (u16 *)D_L16_001FB660[arg0[0x21]])
            return;
    }
}
