/* Near miss from src/levels/shared/L04_00417308-g8.c: the build refuses this body at its
   place — instruction at +0x34 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L04_00417308-g8.c. */
typedef int s32;

extern float D_L04_001B1C8C __attribute__((sda));

extern float D_L04_001B1C90 __attribute__((sda));

extern float D_L04_001B1C94 __attribute__((sda));

extern float D_L04_001B1C98 __attribute__((sda));

extern s32 D_L04_001B1CC4 __attribute__((sda));

extern float func_30EEF8(char *);

extern void func_L04_0046CE68(char *);

void func_L04_0046CD68(char *a0, s32 a1)
{
    char *v1 = *(char **)(a0 + 0x70);
    char *s0 = v1 + 0x70;
    char *p;
    float f;
    float g;

    f = D_L04_001B1C8C;
    *(float *)(v1 + 0x70) = f;
    if (a1 != 0 && D_L04_001B1CC4 != 0)
        *(float *)(v1 + 0x70) = -f;
    *(float *)(s0 + 8) = D_L04_001B1C94;
    *(float *)(s0 + 4) = D_L04_001B1C90;
    *(float *)(s0 + 0xC) = 1.0f;
    *(float *)(s0 + 0x10) = func_30EEF8(s0);
    p = *(char **)(a0 + 0x70);
    g = D_L04_001B1C98;
    *(float *)(p + 0x90) = g;
    if (a1 != 0)
        *(float *)(p + 0x90) = g * 1.1f;
    p = *(char **)(a0 + 0x70);
    *(s32 *)(p + 0x94) = 0;
    *(s32 *)(p + 0x98) = 0;
    func_L04_0046CE68(a0);
}
