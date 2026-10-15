/* Near miss from src/levels/shared/L01_00397110-g8.c: the build refuses this body at its
   place — instruction at +0x88 differs. Kept as a starting point;
   not linked, not counted. */

/* Declaration recovered from nonmatching/levels/func_L01_00397EB0.c. */
extern void func_L01_00397EB0(char *a0);


/* Standalone declarations recovered from src/levels/shared/L01_00397110-g8.c. */
extern float D_L01_001AF50C __attribute__((sda));

extern float D_L01_001AF510 __attribute__((sda));

extern float D_L01_001AF514 __attribute__((sda));

extern float D_L01_001AF528 __attribute__((sda));

extern int D_L01_001AF54C __attribute__((sda));

extern float func_002E3A60(float);

extern float func_002E4460(float, float);

extern int func_003104E8(int);

extern void func_L01_00397A28(char *);

void func_L01_00397870(char *a0)
{
    char *s1 = *(char **)(a0 + 0x68);
    float f;

    *(float *)(a0 + 0x2C) = *(float *)(*(char **)(a0 + 0x24) + 0x24) * D_L01_001AF50C;

    if (*(unsigned char *)(a0 + 0x20) == 1) {
        f = func_002E3A60(*(float *)(a0 + 0xFC));

        *(float *)(a0 + 0x18) =
            *(float *)(s1 + 0x18) + D_L01_001AF510 + D_L01_001AF514 * f;
        *(float *)(a0 + 0xF0) = func_002E4460(*(float *)(a0 + 0xF0), *(float *)(s1 + 0x68));
        *(float *)(a0 + 0xF4) = func_002E4460(*(float *)(a0 + 0xF4), *(float *)(s1 + 0x64));
        *(float *)(a0 + 0xF8) = func_002E4460(*(float *)(a0 + 0xF8), *(float *)(s1 + 0x60));
        *(float *)(a0 + 0xFC) = func_002E4460(
            *(float *)(a0 + 0xFC), D_L01_001AF528 * 0.0002908882452175021f);
    }

    func_L01_00397A28(a0);
    if (func_003104E8(D_L01_001AF54C) == 0)
        func_L01_00397EB0(a0);
}
