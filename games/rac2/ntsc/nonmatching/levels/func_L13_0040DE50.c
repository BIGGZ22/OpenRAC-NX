/* Near miss from src/levels/shared/L13_00406F18-g8.c: the build refuses this body at its
   place — instruction at +0xc differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L13_00406F18-g8.c. */
extern float D_L13_001B1310 __attribute__((sda));

extern int D_L13_001B1300 __attribute__((sda));

extern int D_L13_001B1304 __attribute__((sda));

extern int D_L13_001B1308 __attribute__((sda));

extern int D_L13_001B130C __attribute__((sda));

extern float D_L13_001B1314 __attribute__((sda));

extern char *func_003249F0(int, int);

extern float func_00324B10(void);

extern void func_L13_0040DFC0(char *, char *, char *, char *, int, char *, float, float,
                              float);

void func_L13_0040DE50(char *a0)
{
    char *g;
    float f;

    f = D_L13_001B1310 * *(float *)(a0 + 0x2C);
    g = func_003249F0(0x5F, 0x6F);
    func_L13_0040DFC0(a0 + 0x10, D_L13_001B1308, g, D_L13_001B1300, 2, a0, f,
                      func_00324B10(), 1.0f);

    f = D_L13_001B1310 * *(float *)(a0 + 0x2C) * D_L13_001B1314;
    g = func_003249F0(0x6F, 0x7F);
    func_L13_0040DFC0(a0 + 0x10, D_L13_001B130C, g, D_L13_001B1304, 2, a0, f,
                      func_00324B10(), 1.0f);
}
