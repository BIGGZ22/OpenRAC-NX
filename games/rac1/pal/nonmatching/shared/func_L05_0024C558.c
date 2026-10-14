/* NON_MATCHING func_L05_0024C558 -- src/overlays/shared/help_00237B00.c
 * Best so far: BYTES 28/900 (96.9% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Swing probe on the effect block at g = D_0013E633 + 0xE1D: two calls to 00233EE0 on local vectors, a 4-way flo
 *   hq13/n04: p8.c is BYTES 28/900 (size equal, only register numbers differ; budget spent). What closed the gap, 
 *   Left: x/D load is $v1 where retail uses $a0 (div operand and the lui/lw pair); FP regs: ours f22=load,f24=0.5,
 */
extern void func_L05_00241AA0(void);
extern float func_001F9CE8(void *);
extern float func_L00_001FF860(float, float);
extern int func_L00_0025F410(int);
extern void func_L00_00233EE0(float *, float, float, float);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern int func_L00_001F10E0(float, void *, int, void *);
extern void func_001F9EC0(void *, void *, void *);
extern void func_001FA218(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001FA540(void *, void *, void *);
extern void func_001FA480(void *, void *);
extern void func_001FA460_2(void *, void *) __asm__("func_001FA460");
extern f32 func_001F9D10_1edff8(void *, void *) __asm__("func_001F9D10");
extern s32 func_L00_0020A8B8_20b5b8(s32, f32, f32) __asm__("func_L00_0020A8B8");
extern int D_L05_0015F6B0_m __asm__("D_L05_0015F6B0") MACRO_ADDR;
extern char D_L05_00174340[];
extern u8 D_L05_00174360[];
extern char D_L05_0017AC20[];

typedef int u128_4C558 __attribute__((mode(TI)));

// Runs the swing probe for the effect block: samples two points, then updates the timers and the effect pointer.
void func_L05_0024C558(void) {
    char *g = (char *)D_0013E633 + 0xE1D;
    char *h;
    char *g3;
    char *p;
    float A[4];
    float B[4];
    float C[4];
    float D[16];
    float E[16];
    float F[16];
    float f20;
    float f21;
    float f22;
    float f23;
    float f24;
    float f25;
    float f26;
    float t;
    float fa;
    int x;
    int r;
    int s18;
    int v300;
    int one;

    if (((unsigned char *)g)[0x20AD] == 0) func_L05_00241AA0();
    x = D_L05_0015F6B0_m;
    if (x % 3 == 0) {
        *(float *)(g + 0x248) = 4.0f;
        g[0x254] = 0;
        g[0x255] = 0;
        if (func_L00_0020A8B8_20b5b8((s32)(g + 0x248), 0.7f, 4.0f)) {
            h = D_L05_00174340;
            t = func_001F9CE8(h + 0x40);
            *(float *)(g + 0x250) = func_L00_001FF860(*(float *)(h + 0x48), t);
            if (*(int *)(h + 0x18) != 0) {
                g[0x255] = 1;
                if (func_L00_0025F410(*(int *)(h + 0x18))) g[0x254] = 1;
            }
        }
    }
    x = D_L05_0015F6B0_m;
    if (x % 5 == 0) {
        char *g2 = (char *)D_0013E633 + 0xE1D;
        v300 = *(int *)(g2 + 0x300);
        *(int *)(g2 + 0x260) = 0;
        *(int *)(g2 + 0x258) = 0;
        if (v300 != 0) {
            f24 = *(float *)(g2 + 0x258);
            f21 = 1.1f;
            f23 = 0.5f;
            func_L00_00233EE0(A, f21, f24, 1.0f);
            func_L00_00233EE0(B, f21, f24, -20.0f);
            if (B[2] < f23) B[2] = f23;
            f22 = 20.0f;
            r = func_L00_001EFFF0(A, B, 2, *(int *)(g2 + 0x2080), 0);
            s18 = r;
            if (s18 == 0 || (f22 = func_001F9D10_1edff8(A, D_L05_00174360)) > 3.0f) {
                f25 = f21;
                f20 = -0.4f;
                f21 = f24;
                f26 = 20.0f;
                f24 = 0.1f;
                one = 1;
                do {
                    func_L00_00233EE0(C, f20 + f25, f21, f21);
                    if (func_L00_001F10E0(*(float *)(g2 + 0x234), C, 2, 0) == 0) {
                        *(int *)(g2 + 0x260) = one;
                        if (s18 == 0) *(float *)(g2 + 0x258) = f26;
                        else *(float *)(g2 + 0x258) = f22;
                        {
                            char *g4 = (char *)D_0013E633 + 0xE1D;
                            t = *(float *)(g4 + 0x234) + f20;
                            *(float *)(g4 + 0x25C) = t;
                            if (t < f21) *(int *)(g4 + 0x25C) = 0;
                        }
                        break;
                    }
                    f20 = f20 + f24;
                } while (f20 < f23);
            }
        }
    }
    g3 = (char *)D_0013E633 + 0xE1D;
    if ((unsigned int)(*(int *)(g3 + 0x208C) - 0x15) < 2) {
        if (*(char **)(g3 + 0x86C) != 0) {
            qcopy(*(char **)(g3 + 0x86C) + 0x10, g3 + 0x80);
            *(u128_4C558 *)A = 0;
            A[2] = 0.21f;
            func_001F9EC0(A, A, (void *)(*(int *)(g3 + 0x2080) + 0xC0));
            func_001FA218(E, D_L05_0017AC20);
            func_001F9EE8(A, A, E);
            func_001F9BD8(*(char **)(g3 + 0x86C) + 0x10, *(char **)(g3 + 0x86C) + 0x10, A);
            func_001FA460_2(D, (void *)(*(int *)(g3 + 0x2080) + 0xC0));
            func_001FA540(F, D, E);
            func_001FA480(*(char **)(g3 + 0x86C) + 0xC0, F);
        }
    }
}
