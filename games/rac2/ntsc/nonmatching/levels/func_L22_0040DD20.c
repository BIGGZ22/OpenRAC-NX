/* Near miss from src/levels/22_dobbo_orbit/L22_00358778.c: the build refuses this body at its
   place — instruction at +0x8 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/22_dobbo_orbit/L22_00358778.c. */
typedef int s32;

typedef int TI128 __attribute__((mode(TI)));

extern void func_3300E8();

void func_L22_0040DD20(s32 a0) {
    TI128 buf = 0;

    func_3300E8(a0, &buf);
}
