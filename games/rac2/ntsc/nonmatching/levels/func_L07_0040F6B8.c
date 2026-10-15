/* Near miss from src/levels/07_siberius/L07_003F4578-g8.c: the build refuses this body at its
   place — instruction at +0x30 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/07_siberius/L07_003F4578-g8.c. */
typedef unsigned char      u8;

typedef signed int         s32;

extern u8 *D_L07_001B22DC __attribute__((sda));

extern void func_317520(u8 *a, u8 *b, u8 *c, u8 *d, u8 *e, u8 *f);

void func_L07_0040F6B8(u8 *a0)
{
    u8 *p = *(u8 **)(a0 + 0x68);
    s32 *ids = (s32 *)(p + 0xA8);
    u8 *b = p + 0x160;
    u8 *c = p + 0x120;
    s32 i = 3;

    while (i >= 0) {
        if (*ids != -1) {
            u8 *e = D_L07_001B22DC + (*ids << 8);
            func_317520(e, a0, e + 0x10, e + 0xF0, c, b);
        }
        b += 0x10;
        c += 0x10;
        i--;
        ids++;
    }
}
