/* 0 of 1 places match - last line: 02_maktar_nebula: instruction at +0xac differs
   Size EXACT (196) and 48 of 49 words byte-identical, register allocation included:
   the entry id (0x40 of the 0x68 object, 0xFF guard), the p = table[id] load with the
   table as a SYMBOL (D_L02_001FC060[]; a literal 0x1FC060 compiles to `lui; ori` and
   misses, the symbol gives retail's `lui %hi; addiu %lo`), the u16 walk
   (`q = (u16 *)p`, `e = base + (*q & 0x7FFF) * 256` with base through `$gp`), the four
   tests (e != 0, e[0x20] != 0xFE, != 0xFD, *(s16 *)(e+0xAA) == arg2, e[0x20] == 1),
   the store *(u32 *)(e + 0xB8) = arg1 and `return func_L02_003FB6C0(arg1, e)` - the
   callee takes TWO arguments (the incoming third argument register is dead and gcc
   reuses it for `e`).
   The one word left is the loop-bottom branch: retail is the PLAIN `bgez $v0,-0x48`
   (0x0441FFEE) with `addiu $v1,$v1,0x2` in its delay slot, this cc1 emits `bgezl`
   (0x0443FFEE), i.e. it annuls the delay slot instead of leaving the plain form.
   Measured with seven spellings (`for(;;)` + `if ((s16)*q < 0) break; q++;`,
   `q++; if ((s16)q[-1] < 0) break;`, `*(s16 *)q`, a do-while with a saved `last`,
   `return 1` vs `return f(...)` vs `return r`); every other word does not move, and the
   two shapes that change the increment's position come out 200 bytes. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;
typedef unsigned int u32;

extern u8 *D_L02_00000000 __attribute__((sda));
extern u8 *D_L02_001FC060[];
extern s32 func_L02_003FB6C0(u8 *, u8 *);

s32 func_L02_003FB4D8(u8 *a0, s32 a1, u8 *a2) {
    u8 *base;
    u8 *p;
    u16 *q;
    u8 *e;
    u32 idx;

    idx = *(u32 *)(*(u8 **)(a0 + 0x68) + 0x40);
    if (idx == 0xFF)
        return 0;
    p = D_L02_001FC060[idx];
    if (p == 0)
        return 0;
    q = (u16 *)p;
    base = D_L02_00000000;
    for (;;) {
        e = base + (*q & 0x7FFF) * 256;
        if (e != 0 && e[0x20] != 0xFE && e[0x20] != 0xFD
            && *(s16 *)(e + 0xAA) == a1 && e[0x20] == 1) {
            *(u32 *)(e + 0xB8) = (u32)a0;
            return func_L02_003FB6C0(a0, e);
        }
        if ((s16)*q < 0)
            break;
        q++;
    }
    return 0;
}
