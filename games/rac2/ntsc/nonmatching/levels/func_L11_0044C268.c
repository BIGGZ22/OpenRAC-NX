/* Near miss from src/levels/11_joba/L11_004465F8.c: the build refuses this body at its
   place — 164 bytes, expected 192. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/11_joba/L11_004465F8.c. */
typedef unsigned char      u8;

typedef signed short       s16;

typedef signed int         s32;

extern s32 func_441868(s32 a0, s32 a1);

extern s32 func_33C800(char *a0, s32 a1, s32 a2, s32 a3, float f12, float f13);

void func_L11_0044C268(char *a0)
{
    char *s1 = a0;
    char *s0 = *(char **)(s1 + 0x68);

    if (*(u8 *)(s1 + 0x20) < 4) {
        *(s32 *)(s0 + 0x490) = 0;
        *(s16 *)(s0 + 0x4CA) = 0;
        return;
    }
    if (*(s16 *)(s0 + 0x4CA) != 1) {
        if (func_441868(*(s32 *)(s1 + 0xB8), *(s16 *)(s1 + 0xAA)) == 0) {
            *(s16 *)(s0 + 0x4CA) = 0;
            return;
        }
        *(s16 *)(s0 + 0x4CA) = 1;
    }
    *(s16 *)(s0 + 0x4C8) = func_33C800(s1, *(s32 *)(s0 + 0x4C4), 0, 0, 100.0f, 100.0f);
    *(s32 *)(s0 + 0x490) = *(s32 *)(s0 + 0x220);
}
