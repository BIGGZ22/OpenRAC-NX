/* Near miss from src/levels/03_endako/L03_00399230-g8.c: the build refuses this body at its
   place — 212 bytes, expected 204. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/03_endako/L03_00399230-g8.c. */
typedef unsigned char      u8;

typedef signed int         s32;

typedef float              f32;

extern void func_L03_003CAD78(char *a0);

extern void func_31B920(char *a0, s32 a1, s32 a2);

extern void func_2E77F0(void *a0);

extern f32 D_L03_001B0350[] __attribute__((sda));

void func_L03_003CACA8(char *a0)
{
    char *s0 = *(char **)(a0 + 0x68);

    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(u8 *)(a0 + 0x30) = 0xFF;
        func_31B920(a0, 1, -1);
        *(s32 *)(s0 + 0xC) = 1;
        break;
    case 1: {
        f32 *dst = (f32 *)s0;
        f32 *src = D_L03_001B0350;
        s32 i;

        for (i = 2; i >= 0; i--) {
            f32 f = *dst - *src * 0.016666668f;

            *dst = f;
            if (f < -1.0f)
                *dst = f + 1.0f;
            dst++;
            src++;
        }
        func_2E77F0(func_L03_003CAD78);
        break;
    }
    }
}
