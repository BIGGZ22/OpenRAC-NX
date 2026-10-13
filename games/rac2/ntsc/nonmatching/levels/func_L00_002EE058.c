/* from src/levels/shared/L00_002E5918.c: 6 retail instructions, C compiles to 5 (ld/sd pairs). Missing: retail's two ldc2 quad loads, a COP2 VU0 macro-mode op (.word 0x4BE2086B) and sdc2 quad store, which plain C cannot emit. Not matchable in plain C. */
typedef struct { long long a; long long b; } q128;
void func_L00_002EE058(q128 *dst, q128 *src) {
 *dst = *src;
}
