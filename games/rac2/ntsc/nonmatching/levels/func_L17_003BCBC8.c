/* 0 of 1 places match - last line: 17_smolg: 104 bytes, expected 100
   Picks the ring colour from the shared counter (D_L17_001B2748) plus the
   object's 0xB2 offset, modulo 60, stores it at 0x78, then arms the 0x68
   sub-object (0x8C |= 1, 0x3E |= 8) and clears the object's 0x30 byte. Retail
   divides by 0x3C into $a2, keeps the remainder in $v0, tests `slti $v0,$v0,0xF`
   and selects with `movn $a1,$t0,$v0` (default 0x80404040, then value
   0x80FFFFFF) - note `movn`, not `movz`. This cc1 inverts the test to
   `addiu $v0,$zero,0xE; slt $v0,$v0,$rem; movz` (104 bytes, one extra word), and
   `rem <= 14` or a single ternary make it worse (104/108). The register split
   (v1/a2 for the counter and divisor) also differs. Two spellings kept the same
   shape. Blocker: the movn/movz polarity and the extra constant. */
typedef char *ptr;
typedef unsigned char u8;
typedef unsigned short u16;
typedef short s16;
typedef int s32;

extern s32 D_L17_001B2748;

void func_L17_003BCBC8(char *a0)
{
    int v = D_L17_001B2748 + *(s16 *)(a0 + 0xB2);
    int col = 0x80404040;
    char *a3;

    if (v % 60 < 15)
        col = 0x80FFFFFF;
    *(s32 *)(a0 + 0x78) = col;
    a3 = *(char **)(a0 + 0x68);
    *(s32 *)(a3 + 0x8C) = *(s32 *)(a3 + 0x8C) | 1;
    *(u16 *)(a3 + 0x3E) = *(u16 *)(a3 + 0x3E) | 8;
    *(u8 *)(a0 + 0x30) = 0;
}
