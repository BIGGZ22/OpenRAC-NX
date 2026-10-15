/* NON_MATCHING func_L08_00303998 -- src/overlays/shared/vendor_002D3DF8.c
 * Best so far: SIZE ours 1456 / retail 1440, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 5 runs (not EXACT). Best: p3.c (1456 vs 1440 bytes). A shared moby update (levels 08, 09): reads
 *   Left: the float constant loads come out in another order (retail loads D_L08_00162214 after the state-1/2 bran
 */
extern char *func_0020D348(int);
extern void func_L00_00250800(void *, int, void *);
extern void func_0020DAF8(char *, int, char *);
extern void func_001FA480(void *, void *);
extern void func_L00_00251E30(void *);
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern int func_001FA898(float);
extern float func_L00_001FF860(float, float);
extern void func_L00_0025BBA0(void *, float *, void *, void *);
extern void func_L00_0025D5B0(void *, void *, float, int, int, int);
extern void func_L00_00261B00(void *, int, int, int, int);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_0025E590(void *, void *);
extern float func_002140F8(float, float);
extern float func_001F9878(float);
extern int func_001F9938(void *);
extern s32 func_002140B0(s32);
extern int func_L00_00260FB0_f(char *, void *, float, int, int, void *, int) __asm__("func_L00_00260FB0");
extern float func_001F9D48(void *, void *);
extern float func_001F9B88(float);
extern float func_00214358(void *, int, float);
extern char *D_L08_001B0FB0[];
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern short D_L08_00162210;
extern short D_L08_00162240;
extern short D_L08_00162214;
extern short D_L08_0016221C;
extern short D_L08_00162224;
extern short D_L08_00162220;
extern char D_0013E633[] NOT_SDA;

/* Shared moby update (class 152, levels 08 and 09): steers the moby by its hero-relative vectors and picks its next state. */
void func_L08_00303998(char *m) {
    char *d, *p, *Y, *Y2, *t;
    float S[4];
    float P16[4];
    float fz, q58, q5c;
    int s50, r, x16, st, cls;
    float f, f0, f1, f2, f3, f4, f12, f13;

    d = *(char **)(m + 0x78);
    if (*(unsigned char *)(m + 0x20) == 0) return;
    p = *(char **)(m + 0x24);
    *(float *)(m + 0x2C) = *(float *)(p + 0x24) * *(float *)&D_L08_00162210;
    f2 = *(float *)&D_L08_00162240 * D_0015EE6C;
    *(float *)(d + 0xF4) = f2;

    if (*(char **)(d + 0x1F8) == 0) {
        t = func_0020D348(0x401);
        *(char **)(d + 0x1F8) = t;
        if (t != 0) {
            *(short *)(t + 0x32) = 0x40;
            *(unsigned char *)(*(char **)(d + 0x1F8) + 0x30) = 0x40;
            *(unsigned char *)(*(char **)(d + 0x1F8) + 0x31) = 1;
            *(unsigned short *)(*(char **)(d + 0x1F8) + 0x34) |= 0x100;
            func_L00_00250800(m, 4, S);
            qcopy(*(char **)(d + 0x1F8) + 0x10, S);
        }
        *(unsigned short *)(m + 0x34) |= 0x1000;
    } else {
        func_L00_00250800(m, 4, S);
        func_0020DAF8(m, 4, (char *)P16);
        func_001FA480(*(char **)(d + 0x1F8) + 0xC0, P16);
        qcopy(*(char **)(d + 0x1F8) + 0x10, S);
        func_L00_00251E30(*(char **)(d + 0x1F8));
    }

    fz = 0.0f;
    Y = func_L00_0025B478(m, 0x330000, 0);
    r = func_L00_0025B4D0(m, Y, d + 0x20, 0, &s50, &fz, 0, 4);
    if (s50 != 1 && *(unsigned char *)(m + 0x20) != 0x0B) {
        x16 = 3;
        cls = *(short *)(*(char **)(Y + 0x20) + 0xA6);
        if (cls != 0x47) x16 = r;
        *(float *)(d + 0x20) = *(float *)(d + 0x20) - fz;
        if (*(float *)(d + 0x20) <= 0.0f) x16 = 1;

        r = func_001FA898(512.0f);
        f0 = *(float *)&D_L08_00162214 * D_0015EE70;
        *(int *)(d + 0x90) = r;
        *(float *)(d + 0x98) = 0.5f;
        *(int *)(d + 0x94) = 9;
        *(float *)(d + 0x80) = f0;
        *(unsigned char *)(d + 0xAD) = 0;

        if ((unsigned)x16 < 12) {
            switch (x16) {
            case 0:
            case 11:
                break;
            case 9:
            case 10:
                *(unsigned char *)(d + 0x67) = 0xFA;
                break;
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                f0 = *(float *)&D_L08_0016221C * D_0015EE6C;
                f1 = D_0015EE6C * 3.0f;
                *(float *)(d + 0xC0) = 5.0f;
                *(float *)(d + 0xC4) = 10.0f;
                *(float *)(d + 0x88) = f0;
                *(float *)(d + 0x8C) = f1;
                Y2 = *(char **)(Y + 0x20);
                f12 = *(float *)(m + 0x10) - *(float *)(Y2 + 0x10);
                f13 = *(float *)(m + 0x14) - *(float *)(Y2 + 0x14);
                f0 = func_L00_001FF860(f12, f13);
                qcopy(S, Y + 0x10);
                q58 = f0;
                func_L00_0025BBA0(S, &q58, d + 0x88, d + 0x8C);
                func_L00_0025D5B0(m, d + 0x70, q58, 8, 1, 0);
                *(unsigned char *)(m + 0x20) = 10;
                *(unsigned char *)(d + 0x67) = 0x78;
                break;
            case 1:
            case 2:
                f3 = D_0015EE6C;
                f4 = D_0015EE70 * *(float *)&D_L08_00162214;
                f2 = *(float *)&D_L08_00162224 * f3;
                f1 = *(float *)&D_L08_00162220 * f3;
                *(unsigned short *)(m + 0x34) &= 0xEFFF;
                *(int *)(d + 0x90) = 0x200;
                *(float *)(d + 0x80) = f4;
                *(float *)(d + 0x88) = f2;
                *(float *)(d + 0xC0) = 11.0f;
                *(float *)(d + 0xC4) = 16.0f;
                *(float *)(d + 0x98) = 0.25f;
                *(float *)(d + 0x8C) = f1;
                Y2 = *(char **)(Y + 0x20);
                f12 = *(float *)(m + 0x10) - *(float *)(Y2 + 0x10);
                f13 = *(float *)(m + 0x14) - *(float *)(Y2 + 0x14);
                f0 = func_L00_001FF860(f12, f13);
                qcopy(S, Y + 0x10);
                q5c = f0;
                func_L00_0025BBA0(S, &q5c, d + 0x88, d + 0x8C);
                func_L00_0025D5B0(m, d + 0x70, q5c, 0xC, 1, 0);
                *(unsigned char *)(m + 0x20) = 11;
                *(unsigned char *)(d + 0x67) = 0xF0;
                func_L00_00261B00(m, 2, 3, 0, -1);
                break;
            default:
                break;
            }
        }
        func_L00_0025E4B0(m, (short *)(d + 0x60));
    }
    *(unsigned char *)(m + 0xA4) = 0xFF;
    func_L00_0025E590(m, d + 0x60);

    if (*(int *)(d + 0x38) != 0) {
        f = func_002140F8(180.0f, 240.0f);
        f = func_001F9878(f);
        *(short *)(d + 0x1D8) = (short)func_001FA898(f);
        *(int *)(d + 0x38) = 0;
    }
    func_001F9938(d + 0x1D8);
    f0 = *(float *)(d + 0x1E4);
    if (*(short *)(d + 0x1D8) != 0) f0 = f0 + 6.0f;
    *(float *)(d + 0x1DC) = f0;

    if (func_002140B0(4) == 0) {
        t = D_L08_001B0FB0[*(int *)(d + 0x1D0)];
        r = func_L00_00260FB0_f(m, d + 0x120, *(float *)(d + 0x1DC), 0, 0, t + 0x10, *(int *)t);
        if (r != 2) {
            f0 = func_001F9D48(m + 0x10, d + 0x120);
            if (*(float *)(d + 0x1DC) < f0) {
                *(int *)(d + 0x164) = 2;
            } else {
                f = func_001F9B88(*(float *)(m + 0x18) - *(float *)(d + 0x128));
                if (3.0f < f) *(int *)(d + 0x164) = 2;
            }
        }
    } else {
        p = *(char **)(d + 0x160);
        if (p == 0 || *(unsigned char *)(p + 0x20) == 0xFE || *(unsigned char *)(p + 0x20) == 0xFD) {
            *(char **)(d + 0x160) = 0;
            *(int *)(d + 0x164) = 2;
        } else {
            qcopy(d + 0x120, p + 0x10);
        }
    }
    if (*(int *)(d + 0x160) == 0) *(int *)(d + 0x160) = *(int *)(D_0013E633 + 0x2E9D);

    st = *(unsigned char *)(m + 0x20);
    if (st == 0 || st == 5 || st == 7 || st == 4) {
        f1 = func_00214358(m + 0x10, 0, 0.5f);
        if (f1 < *(float *)(m + 0x18)) *(float *)(m + 0x18) = f1;
    }
}
