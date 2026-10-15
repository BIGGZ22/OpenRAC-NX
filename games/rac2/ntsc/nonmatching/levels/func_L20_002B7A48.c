/* Near miss from src/levels/20_yeedil/L20_002B7A48.c: the build refuses this body at its
   place — 180 bytes, expected 192. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/20_yeedil/L20_002B7A48.c. */
typedef unsigned char      u8;

typedef signed short       s16;

typedef signed int         s32;

extern char D_00189E20[];

extern char D_L20_00139568[];

extern char D_L20_0026D970[];

extern s32 func_2C0C30(s32 a0);

s32 func_L20_002B7A48(char *a0)
{
    char *base = D_00189E20;
    s32 n;

    switch (*(u8 *)(base + 0x22B4)) {
    case 1:
        return 0;
    case 4:
        return 0x51;
    }
    if (a0 != 0 && *(u8 *)(base + 0x22BA) != 0 && *(s16 *)(base + 0x24EC) == 0) {
        n = *(u8 *)(D_L20_00139568 + func_2C0C30(0));
        return *(s32 *)(D_L20_0026D970 + n * 0xE0 + 0x28);
    }
    if (*(s32 *)(base + 0x24CC) == 1)
        return 0x51;
    return *(u8 *)(base + 0x1492) ? 0x67 : 0;
}
