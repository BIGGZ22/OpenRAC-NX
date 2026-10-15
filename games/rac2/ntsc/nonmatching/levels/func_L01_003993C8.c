/* Near miss from src/levels/shared/L01_00397110-g8.c: the build refuses this body at its
   place — instruction at +0x4 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110-g8.c. */
extern float D_L01_001AF638 __attribute__((sda));

extern float D_L01_001AF63C __attribute__((sda));

extern float D_L01_001B2EF0 __attribute__((sda));

extern float D_L01_001B2F00 __attribute__((sda));

extern float D_001B2EF4 __attribute__((nosda));

extern int D_001B2F04 __attribute__((nosda));

extern float func_00310580(float, float);

extern float func_002E35D0(char *);

extern float func_002E35A0(float *);

void func_L01_003993C8(int a0)
{
    float s, x, lo, hi;

    if (a0 == 0) {
        s = 5.0f;
        x = 2.0f;
        hi = 8.0f;
        lo = 4.0f;
    } else {
        s = 2.5f;
        x = 20.0f;
        hi = 3.0f;
        lo = 2.0f;
    }

    if (D_L01_001AF638 >= 0.0f)
        lo = D_L01_001AF638;
    if (D_L01_001AF63C >= 0.0f)
        hi = D_L01_001AF63C;

    {
        float k = s * 0.01666666753590107f;
        float v[4];

        D_L01_001B2EF0 = func_00310580(-k, k);
        D_001B2EF4 = func_00310580(-k, k);
        D_L01_001B2F00 = func_00310580(x, 10.0f);
        v[0] = k;
        v[1] = k;
        D_001B2F04 = (int)((func_002E35D0((char *)&D_L01_001B2EF0) /
                            func_002E35A0(v)) *
                               (hi - lo) +
                           lo);
    }
}
