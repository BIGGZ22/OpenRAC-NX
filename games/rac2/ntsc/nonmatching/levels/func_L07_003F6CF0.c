/* Near miss from src/levels/07_siberius/L07_003F4578-g8.c: the build refuses this body at its
   place — 220 bytes, expected 208. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/07_siberius/L07_003F4578-g8.c. */
typedef unsigned char      u8;

typedef unsigned short     u16;

typedef unsigned int       u32;

typedef signed int         s32;

typedef float              f32;

extern void func_3109C8(u8 *a, s32 b, s32 c);

extern s32 func_312FA8(u8 *a, s32 b, s32 c);

extern f32 func_313008(u8 *a, s32 b);

extern void func_313DD8(void *a, f32 b, f32 c);

extern f32 D_L07_001B0FD0 __attribute__((sda));

void func_L07_003F6CF0(u8 *a0)
{
    u8 *s1 = *(u8 **)(a0 + 0x68);

    if (*(u8 *)(a0 + 0x20) == 0) {
        *(u32 *)(s1 + 0x30) = 0;
        *(f32 *)(a0 + 0x2C) = *(f32 *)(*(u8 **)(a0 + 0x24) + 0x24) * D_L07_001B0FD0;
        func_3109C8(a0, 1, -1);
        *(u32 *)(a0 + 0x98) = 0;
        *(u16 *)(a0 + 0x34) = (*(u16 *)(a0 + 0x34) | 0x41) & 0xEFFF;
        return;
    }
    func_313DD8(s1 + 0x20, 0.0f, func_313008(a0, func_312FA8(a0, 0x10000, 0)));
    if (*(f32 *)(s1 + 0x20) <= 0.0f)
        func_3109C8(a0, 3, -1);
    *(u8 *)(a0 + 0xA8) = 0xFF;
}
