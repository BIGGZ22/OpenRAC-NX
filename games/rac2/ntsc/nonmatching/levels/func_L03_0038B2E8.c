/* Near miss from src/levels/shared/L03_002AB5C8.c: the build refuses this body at its
   place — instruction at +0x8 differs. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/shared/L03_002AB5C8.c. */
typedef int TI128 __attribute__((mode(TI)));

extern TI128 D_001A8F20;

extern void func_2EEFB0(void *, void *, void *);

extern void func_2EF020(void *, float);

void func_L03_0038B2E8(unsigned char *a0, unsigned char *a1, int a2) {
    unsigned char *p;

    *(TI128 *)a0 = D_001A8F20;
    if (a1 == 0)
        return;
    if (a2 == 0)
        return;
    {
        float n = (float)a2;
        int i;

        p = a1 + 0x10;
        for (i = 0; i < a2; i++) {
            func_2EEFB0(a0, a0, p);
            p += 0x10;
        }
        func_2EF020(a0, 1.0f / n);
    }
}
