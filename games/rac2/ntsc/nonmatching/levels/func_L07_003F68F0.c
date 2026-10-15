/* Near miss from src/levels/07_siberius/L07_003F4578-g8.c: the build refuses this body at its
   place — 128 bytes, expected 108. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/07_siberius/L07_003F4578-g8.c. */
typedef unsigned char      u8;

typedef float              f32;

extern f32 func_2E5318(f32 a, f32 b);

extern f32 func_2E4918(f32 a, f32 b);

extern f32 D_L07_001B0F84 __attribute__((sda));

void func_L07_003F68F0(u8 *a0, f32 x)
{
    u8 *s0 = *(u8 **)(a0 + 0x68);
    f32 v;
    f32 r;

    v = func_2E5318(*(f32 *)(s0 + 0x54), x);
    *(f32 *)(s0 + 0x54) = v;
    r = func_2E4918(v, x);
    v = D_L07_001B0F84 * (r * 0.0174532925f);
    *(f32 *)(a0 + 0xF8) = v;
    *(f32 *)(a0 + 0xF8) = func_2E5318(v, *(f32 *)(s0 + 0x60));
}
