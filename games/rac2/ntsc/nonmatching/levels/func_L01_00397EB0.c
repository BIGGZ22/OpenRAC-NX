/* Near miss from src/levels/shared/L01_00397110-g8.c: the build refuses this body at its
   place — instruction at +0x18 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L01_00397110-g8.c. */
extern int func_003104E8(int);

extern void func_002E3470(void *, void *, void *);

extern void func_L01_00397EB0(char *);

extern float D_L01_001AF554 __attribute__((sda));

extern float D_L01_001AF558 __attribute__((sda));

extern float D_L01_001AF55C __attribute__((sda));

extern float D_L01_001AF560 __attribute__((sda));

extern unsigned char D_L01_001AF564 __attribute__((sda));

extern int D_L01_001AF568 __attribute__((sda));

extern int D_L01_001AF56C __attribute__((sda));

extern float D_L01_001AF570 __attribute__((sda));

extern float D_L01_001AF574 __attribute__((sda));

extern float D_L01_001AF578 __attribute__((sda));

extern float D_L01_001AF598 __attribute__((sda));

extern float D_00189F08 __attribute__((nosda));

extern void func_002E34E0(void *, char *, float);

extern void func_00307390(char *, int, void *);

extern void func_00310708(void *, float, float);

extern void func_00330048(void *, void *, int, int, unsigned char, int);

void func_L01_00397EB0(char *a0)
{
    float m[4];
    float m2[4];
    float tmp[4];
    float *p;
    int i;

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        func_002E34E0(tmp, a0, 0.0009765625f);
    } else {
        func_00307390(a0, 0, tmp);
        if (tmp[2] < D_00189F08)
            return;
    }

    {
        float k = 0.01666666753590107f;

        func_00310708(m, D_L01_001AF554 * k, D_L01_001AF558 * k);
        p = m;
        {
            unsigned char a = D_L01_001AF564;

            func_00330048(tmp, p, D_L01_001AF568, a, (unsigned char)func_003104E8(0xFF), 0);
        }

        i = 2;
        do {
            func_00310708(m2, D_L01_001AF55C * k, D_L01_001AF560 * k);
            func_002E3470(p, p, m2);
            {
                unsigned char a = D_L01_001AF564;

                func_00330048(tmp, p, D_L01_001AF56C, a,
                              (unsigned char)func_003104E8(0xFF), 0);
            }
        } while (--i >= 0);
    }
}
