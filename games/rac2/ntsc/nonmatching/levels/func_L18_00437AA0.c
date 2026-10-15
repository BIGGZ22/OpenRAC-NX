/* Near miss from src/levels/18_damosel/L18_00433460-g8.c: the build refuses this body at its
   place — 248 bytes, expected 260. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_00433460-g8.c. */
typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

typedef int TI128 __attribute__((mode(TI)));

extern u16 D_L18_001B18FC __attribute__((sda));

extern float D_L18_001B191C __attribute__((sda));

extern void *func_0029EFD0(short a);

extern float func_00283248(float a, float b);

extern void func_002A0F48(void *a);

void *func_L18_00437AA0(u8 *obj, void *pa1, void *pa2, s32 pa3, float f1, float f2)
{
    u8 *s1 = func_0029EFD0(0xF9E);
    u8 *s0;
    u8 *d1;
    u8 *d2;

    if (s1 != 0) {
        s0 = *(u8 **)(s1 + 0x68);
        *(u16 *)(s1 + 0x32) = 0xFF;
        *(u8 *)(s1 + 0x31) = 1;
        *(u8 *)(s1 + 0x30) = 0xFF;
        *(u8 *)(s1 + 0x20) = 0;
        *(s32 *)(s0 + 0x24) = (s32)obj;
        d1 = s1 + 0x10;
        *(TI128 *)d1 = *(TI128 *)pa1;
        d2 = s0 + 0x10;
        *(TI128 *)d2 = *(TI128 *)pa2;
        *(u16 *)(s0 + 0x28) = D_L18_001B18FC;
        *(float *)(s0 + 0x1C) = f1;
        *(s32 *)(s0 + 0x20) = pa3;
        *(u16 *)(s0 + 0x2A) = 0;
        *(s32 *)(s1 + 0xF0) = 0;
        *(s32 *)(s1 + 0xF4) = 0;
        *(float *)(s1 + 0xF8) = func_00283248(*(float *)pa2, *(float *)((u8 *)pa2 + 4));
        *(float *)(s0 + 0x30) = f2;
        *(float *)(s1 + 0x2C) = *(float *)(s1 + 0x2C) * D_L18_001B191C;
        func_002A0F48(s1);
    }
    return s1;
}
