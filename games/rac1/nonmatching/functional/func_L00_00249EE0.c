/* functional: medium. Menu hit test: for selection b < 0x9D the screen coordinate x must lie in [73.5, 80.0] and D_L00_001845C0 be nonzero; otherwise x in [51.5, 54.0] and D_L00_001845B8 nonzero. Includes the pieces after it (return 1 at 0x249F38, func_L00_00249F40). */
extern int D_L00_001845C0_49EE0 __asm__("D_L00_001845C0");
extern int D_L00_001845B8_49EE0 __asm__("D_L00_001845B8");

int func_L00_00249EE0(int a, int b, float x) {
    if (b < 0x9D) {
        if (x < 73.5f) return 0;
        if (80.0f < x) return 0;
        return D_L00_001845C0_49EE0 != 0;
    }
    if (x < 51.5f) return 0;
    if (54.0f < x) return 0;
    return D_L00_001845B8_49EE0 != 0;
}
