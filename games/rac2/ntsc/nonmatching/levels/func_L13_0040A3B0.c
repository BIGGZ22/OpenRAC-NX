/* Near miss from src/levels/shared/L13_00406F18.c: the build refuses this body at its
   place — instruction at +0x0 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L13_00406F18.c. */
extern float func_0032E870(char *, void *, char *);

extern float func_003281B8(void *, char *, int, float, float, float, float);

extern void func_002F8FA0(void *, void *);

extern void func_002F92E8(char *, char *, void *);

extern float func_002F86B0(float);

typedef int TI128 __attribute__((mode(TI)));

float func_L13_0040A3B0(char *a0, char *a1, char *a2, char *a3)
{
    float tmp[4];
    float out[4];
    float f;

    *(TI128 *)tmp = *(TI128 *)a2;
    f = func_0032E870(a0, tmp, a3);
    {
        int st = 0;

        func_003281B8(&st, a1 + 0xB4, 0, f, 0.0034906589426100254f,
                      0.0034906589426100254f, 0.29786956310272217f);
    }
    *(TI128 *)tmp = 0;
    tmp[2] = *(float *)(a1 + 0xB4);
    func_002F8FA0(out, tmp);
    func_002F92E8(a0 + 0xC0, a0 + 0xC0, out);
    f = func_002F86B0(f);
    return f - func_002F86B0(*(float *)(a1 + 0xB4));
}
