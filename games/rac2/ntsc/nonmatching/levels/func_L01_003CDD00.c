/* Near miss from src/levels/01_oozla/L01_003CAFE0.c: the build refuses this body at its
   place — 352 bytes, expected 344. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/01_oozla/L01_003CAFE0.c. */
typedef unsigned char  u8;

typedef unsigned short u16;

typedef unsigned int   u32;

typedef signed int     s32;

typedef float          f32;

typedef int TI128 __attribute__((mode(TI)));

extern s32 func_2E45B8(f32 a0);

extern void func_3377C8(u32 a0);

extern void func_310060(u8 *a0, s32 a1, s32 a2);

extern void func_310418(u8 *a0, s32 a1, s32 a2);

extern u8 *D_L01_001B255C;

extern u32 D_L01_001FFE80[];

void func_L01_003CDD00(u8 *a0)
{
    u8 *s0 = *(u8 **)(a0 + 0x68);

    *(u8 *)(s0 + 0x48) = func_2E45B8(30.0f);
    *(u8 *)(s0 + 0x4A) = func_2E45B8(24.0f);
    *(f32 *)(a0 + 0x2C) = *(f32 *)(*(u8 **)(a0 + 0x24) + 0x24);
    *(TI128 *)(s0 + 0x60) = *(TI128 *)(a0 + 0x10);
    *(u8 *)(s0 + 0x2E) = 3;
    *(f32 *)(s0 + 0x30) = 3.0f;
    *(u8 *)(s0 + 0x2A) = 8;
    *(u8 *)(s0 + 0x28) = 3;
    *(u32 *)(s0 + 0xB4) = 0;
    *(u32 *)(s0 + 0xC0) = -1;
    if (*(s32 *)(s0 + 0xCC) != -1) {
        u8 *e = D_L01_001B255C + (*(s32 *)(s0 + 0xCC) << 8);

        if (*(u8 *)(e + 0x20) != 0xFE) {
            if (*(u8 *)(e + 0x20) != 0xFD)
                *(f32 *)(*(u8 **)(s0 + 0xA8) + 0x18) = *(f32 *)(a0 + 0x18);
        }
    }
    if (*(u32 *)(s0 + 0xA8) != -1)
        func_3377C8(D_L01_001FFE80[*(u32 *)(s0 + 0xA8)]);
    if (*(u32 *)(s0 + 0xAC) != 0) {
        *(u16 *)(a0 + 0x34) &= 0xEFFF;
        func_310060(a0, 0, 0);
        *(u32 *)(a0 + 0x48) = 0;
        func_310418(a0, 0, 4);
        return;
    }
    *(u16 *)(a0 + 0x34) |= 0x1000;
    func_310060(a0, 1, 0);
    func_310418(a0, 0, 1);
}
