/* Near miss from src/levels/18_damosel/L18_003F80A0.c: the build refuses this body at its
   place — 196 bytes, expected 220. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_003F80A0.c. */
typedef int TI128 __attribute__((mode(TI)));

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef int s32;

extern float func_00308A98();

extern int *D_L18_001FE480[];

s32 func_L18_0042F688(u8 *a0, u8 *a1, s32 a2, s16 *a3, float f12, s32 a4) {
    u8 *s1 = (u8 *)D_L18_001FE480[a2];
    s32 idx = *a3;

    *(TI128 *)a1 = *(TI128 *)(s1 + (idx << 4) + 0x10);
    if (a4 != 0) {
        if (func_00308A98(a0 + 0x10) < f12) {
            *(u16 *)a3 = (u16)(*(u16 *)a3 - 1);
        }
        if (*a3 >= 0) return 1;
        return 0;
    }
    if (func_00308A98(a0 + 0x10) < f12) {
        *(u16 *)a3 = (u16)(*(u16 *)a3 + 1);
    }
    return *a3 < *(s32 *)s1;
}
