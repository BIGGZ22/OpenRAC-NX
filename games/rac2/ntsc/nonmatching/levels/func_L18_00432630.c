/* Near miss from src/levels/18_damosel/L18_003F80A0.c: the build refuses this body at its
   place — 92 bytes, expected 100. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_003F80A0.c. */
typedef unsigned char u8;

typedef short s16;

extern float func_00308FB0(float arg0, float arg1);

extern float func_003099E8(float arg0, float arg1);

float func_L18_00432630(u8 *a0, float *a1) {
    u8 *v1 = *(u8 **)(a0 + 0x68);
    s16 v0 = *(s16 *)(v1 + 0x7FC);
    u8 *s0 = *(u8 **)(v1 + 0x7C0);
    float f;

    (void)v0;
    f = func_00308FB0(a1[0] - *(float *)(s0 + 0x10),
                      a1[1] - *(float *)(s0 + 0x14));
    return func_003099E8(*(float *)(s0 + 0xF8), f) - 1.5707964f;
}
