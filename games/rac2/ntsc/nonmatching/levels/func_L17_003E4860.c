/* 0 of 1 places match - last line: 17_smolg: 120 bytes, expected 124
   Candidate is four bytes short.  Walks the halfword list the object's byte 0x21 selects
   from table 0x001FB720, and for every entry whose record byte 0x20 is non-negative and
   whose halfword 0xAA is 0xB88 arms the record's word at 0x68 + 0x406.  Differences: the
   byte/index registers are swapped ($v0/$v1), the loop pointer lands in $a0 here against
   retail's $a1 with the record base in $a2, and this cc1 emits the plain `bltz`/`bne`
   where retail annuls them (`bltzl`/`bnel`, the next `lh` in each delay slot) - the
   branch-likely wall of the wave-5 near misses.  Tried and identical: the record base
   hoisted into a named local before the loop (which does give the `$gp` form of the base
   load, in retail's delay slot).  Needs cc1 to annul those two branches. */

typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef short s16;
typedef char s8;

extern s16 *D_L17_001FB720[];
extern s32 D_L17_001B2C1C __attribute__((sda));

void func_L17_003E4860(s32 a0) {
    s32 i = *(u8 *)(a0 + 0x21);
    s16 *p;
    u8 *base;

    if (i == 0xFF)
        return;
    p = D_L17_001FB720[i];
    if (p == 0)
        return;
    base = (u8 *)D_L17_001B2C1C;
    do {
        u8 *q = base + ((*(u16 *)p & 0x7FFF) << 8);

        if (*(s8 *)(q + 0x20) >= 0 && *(s16 *)(q + 0xAA) == 0xB88)
            *(s16 *)(*(s32 *)(q + 0x68) + 0x406) = 1;
        p++;
    } while (*p >= 0);
}
