/* NON_MATCHING func_L02_002EB648 -- src/overlays/shared/vendor_002A5218.c
 * Best so far: SIZE ours 1472 / retail 1468, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 10 runs (budget spent). Best candidate p7.c: SIZE 1472 vs 1468, case 3 branch layout and the pro
 *   Unblock: the scheduler's placement of the case 0 sb/lwc1 pair and of the D_L02_00179A90 test in case 4; a diff
 */
extern char D_0013E633[];
extern char D_0013D5EB[];
extern float D_L02_001744E8;
extern float D_L02_0015F4FC MACRO_ADDR;
extern int D_L02_0015F6B0 MACRO_ADDR;
extern int D_L02_0015F6A8 MACRO_ADDR;
extern int D_L02_0015F720 MACRO_ADDR;
extern int D_L02_00179A90 MACRO_ADDR;
extern char *D_L02_00167480;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern int func_001F9850(int);
extern void func_L02_002EBC08(char *moby);
extern float func_00214358(void *, int, float);
extern float func_001FA748(float, float);
extern float func_001FA790(float, float);
extern void func_L02_002EBCE8(char *m);
extern float func_001F9D48(void *, void *);
extern float func_001F9B88(float);
extern void func_L00_00299B68(int);
extern void func_L00_002618D8(int, int);
extern void func_L00_00264DB8(int, int);
extern void func_L00_00203FB8(void);
extern int func_L00_00203F20(int, int);
extern void func_L00_00217718(void *, void *, int, int);
extern int func_0020BFC8(int, int);
extern void func_0020D678(void *);
extern float func_00214D88(float *, float *, float, float, float, float);
extern void func_001F9C08(void *, void *, void *, float);
extern void func_L00_002EBF50(void *, void *, int, int, int);
extern void func_L00_002EBE88(void *);
extern void func_L00_002EBEE0(void *);
extern float func_00214D28(float *, float, float);
extern void func_L00_002EC0C8(int);
typedef int u128_002EB648 __attribute__((mode(TI)));

/* Update function of moby class 1005 (tresspasser) on level 02, class 1016 (hydrodisplacer) on level 06: state machine. */
void func_L02_002EB648(char *moby) {
    char *d;
    char *g;
    char *gp;
    unsigned char *q;
    int *e;
    int sel;
    float r;
    float t;
    float e70;
    float e6c;

    sel = 0x16;
    d = *(char **)(moby + 0x78);
    r = 3.1415927f / (float)func_001F9850(0x3C);
    if (*(short *)(moby + 0xA6) == 0x3ED) {
        sel = 0x1A;
    }

    switch (*(unsigned char *)(moby + 0x20)) {
    case 0:
        func_L02_002EBC08(moby);
        q = (unsigned char *)(D_0013D5EB + 5);
        if (q[sel] != 0) {
            func_0020D678(moby);
            break;
        }
        func_00214358(moby + 0x10, 0, 0.5f);
        *(float *)(moby + 0x44) = 3.1415927f;
        *(float *)(moby + 0x40) = 1.5707964f;
        moby[0x20] = 1;
        *(float *)(moby + 0x18) = D_L02_001744E8 + 0.2f;
        break;
    case 1:
        *(float *)(moby + 0x48) = func_001FA748(*(float *)(moby + 0x48), r);
        func_L02_002EBCE8(moby);
        if (D_L02_0015F6B0 % 10 != 0) {
            break;
        }
        g = D_0013E633 + 0xE9D;
        if (!(func_001F9D48(moby + 0x10, g) < 1.0f)) {
            break;
        }
        t = func_001F9B88(*(float *)(moby + 0x18) - *(float *)(g + 0x8));
        if (!(t < 2.0f)) {
            break;
        }
        moby[0x20] = 3;
        *(unsigned short *)(moby + 0x34) |= 1;
        moby[0x31] = 0;
        *(int *)(moby + 0x94) = 0;
        if (*(short *)(moby + 0xA6) == 0x3ED) {
            func_L00_00299B68(4);
        } else {
            func_L00_00299B68(2);
        }
        break;
    case 2:
        break;
    case 3:
        if (D_L02_0015F6A8 == 2) {
            break;
        }
        if (*(unsigned char *)(D_0013E633 + 0x2EC1) != 0) {
            if (sel == 0x16) {
                func_L00_002618D8(0x16, 0);
                D_L02_0015F720 = 0;
            } else {
                func_L00_002618D8(sel, 0);
            }
        } else {
            func_L00_002618D8(sel, 1);
        }
        if (*(short *)(moby + 0xA6) == 0x3ED) {
            func_L00_00264DB8(0x7DF, -1);
            func_L00_00203FB8();
            func_L00_00203F20(0x7D9, 0x13);
            D_L02_0015F4FC = 1.0f;
            moby[0x20] = 4;
            func_L00_00217718(D_0013E633 + 0xE9D, D_0013E633 + 0xE9D + 0x10, 0x72, 1);
        } else {
            func_0020BFC8(0, -1);
            func_0020D678(moby);
        }
        break;
    case 4:
        {
            float a[16];
            float o[4];
            float w[4];
            *(u128_002EB648 *)&a[0] = 0;
            *(u128_002EB648 *)&a[4] = 0;
            *(u128_002EB648 *)&a[8] = 0;
            *(u128_002EB648 *)&a[12] = 0;
            a[0] = 91.2f;
            a[1] = 270.76f;
            a[2] = 64.85f;
            a[4] = 106.98f;
            a[5] = 273.46f;
            a[6] = 62.4f;
            a[9] = 0.33f;
            a[10] = 0.55f;
            a[12] = 0.07f;
            a[13] = 0.43f;
            a[14] = 0.72f;
            a[8] = 0.07f;
            e70 = D_0015EE70 * 0.15f;
            e6c = D_0015EE6C * 0.15f;
            func_00214D88((float *)(d + 0x40), (float *)(d + 0x44), 1.0f, e70, e70, e6c);
            func_001F9C08(o, a, a + 4, *(float *)(d + 0x40));
            t = func_001FA790(a[14], a[10]);
            w[2] = func_001FA748(t * *(float *)(d + 0x40), a[10]);
            t = func_001FA790(a[13], a[9]);
            w[1] = func_001FA748(t * *(float *)(d + 0x40), a[9]);
            t = func_001FA790(a[12], a[8]);
            w[0] = func_001FA748(t * *(float *)(d + 0x40), a[8]);
            gp = D_L02_00167480;
            if (gp != 0 && *(short *)(gp + 0x86) != 5) {
                func_L00_002EBF50(o, w, 1, 0, 0);
            }
            func_L00_002EBE88(o);
            func_L00_002EBEE0(w);
            e = &D_L02_00179A90;
            if (D_L02_00179A90 == 0 && e[9] == -1 && *(float *)(d + 0x40) > 0.2f) {
                func_00214D28(&D_L02_0015F4FC, 1.0f, D_0015EE6C * 4.0f);
            } else {
                func_00214D28(&D_L02_0015F4FC, 0.0f, D_0015EE6C * 4.0f);
            }
            if (D_L02_0015F4FC == 1.0f) {
                g = D_0013E633 + 0xE9D;
                func_L00_00217718(g, g + 0x10, 0, 0);
                func_L00_002EC0C8(0);
                moby[0x20] = 5;
            }
        }
        break;
    case 5:
        func_00214D28(&D_L02_0015F4FC, 0.0f, D_0015EE6C * 4.0f);
        if (D_L02_0015F4FC == 0.0f) {
            func_0020BFC8(0, -1);
            func_0020D678(moby);
        }
        break;
    }
}
