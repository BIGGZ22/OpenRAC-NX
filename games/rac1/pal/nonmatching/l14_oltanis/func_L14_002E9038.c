/* NON_MATCHING func_L14_002E9038 -- src/overlays/l14_oltanis/vendor_002E0538.c
 * Best so far: SIZE ours 1188 / retail 1180, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Oltanis shuttle update (class 557): reads the route record at d+0x150 and eases d+0xFC toward a target speed. 
 *   Left: the frame differs (retail keeps sp+0x10..0x50 vectors in s-registers and copies moby+0x40 before the mob
 */
extern int func_00215570(void *, int);
extern void func_0020D960(char *, int, void *);
extern int func_001E9730();
extern void func_0020D678(void *);
extern void func_L01_002F6540(char *);
extern void func_L00_00250800(void *, int, void *);
extern void func_001FA218(void *, void *);
extern void func_001F9EE8(void *, void *, void *);
extern void func_001F9BD8(void *, void *, void *);
extern int func_L00_0028EB98(void *, int);
extern int func_0022ED80(int, int, int);
extern float func_001FA748(float, float);
extern void func_L00_001FFED8(void *, int, float);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_002617B0(char *, void *, void *, void *);
extern int func_001F9850(int);
extern float func_001FA888(int);
extern float func_001F9CB8(void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern int func_001F9938(void *);
extern char *D_L14_00160098 MACRO_ADDR;
extern char *D_L14_001B0F30[];
extern char D_L14_001FCBB0[];
extern float D_0015EE6C MACRO_ADDR;
extern char D_0013E633[];
extern short D_L14_00161C5C;
extern short D_L14_00161C58;
extern short D_L14_00161C60;

typedef int u128_002E9038 __attribute__((mode(TI)));

/* Oltanis shuttle update (moby class 557): steers the shuttle along its route and eases its speed toward the target. */
void func_L14_002E9038(char *moby) {
    char *d;
    char *q;
    char *other;
    char *p5;
    u128_002E9038 s0, s10, s20, s30, s40, s50;
    char *sp0 = (char *)&s0;
    char *sp10 = (char *)&s10;
    char *sp20 = (char *)&s20;
    char *sp30 = (char *)&s30;
    char *sp40 = (char *)&s40;
    char *sp50 = (char *)&s50;
    char *tmp;
    int r;
    int i;

    if (moby == 0) {
        return;
    }
    d = *(char **)(moby + 0x78);
    if (d == 0) {
        return;
    }

    if (*(int *)(d + 0x21C) >= 0 && func_00215570(D_0013E633 + 0xE9D, *(int *)(d + 0x21C)) != 0) {
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 0x41;
        *(int *)(moby + 0x94) = 0;
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) | 0x41;
        if (*(int *)(d + 0x150) < 0) {
            return;
        }
        {
            char *o = D_L14_00160098 + (*(int *)(d + 0x150) << 8);
            *(unsigned short *)(o + 0x34) = (*(unsigned short *)(o + 0x34) | 0x41) & 0xEFFF;
            *(int *)(o + 0x94) = 0;
        }
        return;
    }

    if (*(int *)(moby + 0x94) == 0) {
        *(unsigned short *)(moby + 0x34) = *(unsigned short *)(moby + 0x34) & 0xFFBE;
        *(int *)(moby + 0x94) = *(int *)(*(char **)(moby + 0x24) + 0x10);
        if (*(int *)(d + 0x150) >= 0) {
            char *o = D_L14_00160098 + (*(int *)(d + 0x150) << 8);
            unsigned short w = *(unsigned short *)(o + 0x34) & 0xFFBE;
            char *src;
            *(unsigned short *)(o + 0x34) = w;
            src = *(char **)(o + 0x24);
            w = w | 0x1000;
            *(int *)(o + 0x94) = *(int *)(src + 0x10);
            *(unsigned short *)(o + 0x34) = w;
        }
    }

    p5 = moby + 0x10;
    *(u128_002E9038 *)sp0 = *(u128_002E9038 *)p5;
    *(u128_002E9038 *)sp10 = *(u128_002E9038 *)(moby + 0x40);
    if (moby[0x20] == 0) {
        tmp = D_L14_001B0F30[*(int *)(d + 0x74)];
        *(u128_002E9038 *)p5 = *(u128_002E9038 *)(tmp + 0x10);
        *(int *)(d + 0x200) = 0;
        *(int *)(d + 0x15C) = -1;
        *(unsigned short *)(d + 0x206) = 0;
        func_0020D960(moby, 0, d + 0x1C0);
        if (*(int *)(d + 0x150) != -1) {
            *(unsigned short *)(d + 0x204) = 0;
            *(float *)(d + 0x214) = *(float *)(d + 0xFC);
        } else {
            func_001E9730(D_L14_001FCBB0, *(short *)(moby + 0xB2));
            func_0020D678(moby);
            return;
        }
    }

    func_L01_002F6540(moby);
    other = D_L14_00160098 + (*(int *)(d + 0x150) << 8);
    if (*(int *)(d + 0x158) == 0x3039) {
        if (*(int *)(d + 0x154) == -1) {
            *(u128_002E9038 *)sp30 = *(u128_002E9038 *)p5;
            tmp = sp30;
        } else {
            func_L00_00250800(moby, *(int *)(d + 0x154), sp30);
            tmp = sp30;
        }
        func_001FA218(sp50, moby + 0x40);
        func_001F9EE8(sp40, d + 0x160, sp50);
        func_001F9BD8(other + 0x10, tmp, sp40);
    }

    if (*(int *)(d + 0x15C) != -1) {
        if (func_L00_0028EB98(moby, *(int *)(d + 0x15C)) != 0) {
            goto L92A8;
        }
        if (*(int *)(d + 0x15C) != -1) {
            goto L92A8;
        }
    }
    *(int *)(d + 0x15C) = func_0022ED80(0, 4, (int)moby);

L92A8:
    *(float *)(d + 0x200) = func_001FA748(*(float *)(d + 0x200), 0.1745329201221466f);
    func_L00_001FFED8(d + 0x1D0, 0, *(float *)(d + 0x200));
    func_001F9BF0(sp20, p5, sp0);
    func_L00_002617B0(d + 0x180, sp20, sp10, moby + 0x40);

    q = D_0013E633 + 0xE1D;
    if (*(int *)(q + 0x208C) == 13 && *(int *)(q + 0x964) == (int)other) {
        if (*(short *)(d + 0x204) == 0) {
            r = func_001F9850(*(int *)&D_L14_00161C5C);
            *(short *)(d + 0x204) = 1;
            *(short *)(d + 0x20C) = (short)r;
            *(float *)(d + 0x210) = 1.0f / func_001FA888((short)r);
        }
        goto L946C;
    }
    if (*(short *)(d + 0x204) != 1) {
        goto L9430;
    }
    if (*(short *)(q + 0x30E) != 0 && *(float *)(q + 0x88) > *(float *)(moby + 0x18) - 3.0f) {
        goto L93D8;
    }
    goto L93A4;

L93D8:
    *(u128_002E9038 *)sp30 = *(u128_002E9038 *)(q + 0x80);
    func_001F9BF0(sp20, other + 0x10, sp30);
    *(int *)(sp20 + 8) = 0;
    {
        float c = func_001F9CB8(sp20);
        float lim = *(float *)&D_L14_00161C58 * D_0015EE6C;
        if (lim < c) {
            c = lim;
        }
        func_L00_001FF4B0(q + 0xF0, sp20, c);
    }
    goto L942C;

L93A4:
    r = func_001F9850(*(int *)&D_L14_00161C60);
    *(short *)(d + 0x204) = 0;
    *(short *)(d + 0x20C) = (short)r;
    *(float *)(d + 0x210) = 1.0f / func_001FA888((short)r);
    *(float *)(d + 0x218) = *(float *)(d + 0xFC);
    goto L942C;

L942C:
    if (*(short *)(d + 0x204) != 0) {
        goto L946C;
    }
L9430:
    if (*(short *)(d + 0x204) != 0) {
        goto L946C;
    }
    func_001F9938(d + 0x20C);
    {
        float f0 = func_001FA888(*(short *)(d + 0x20C));
        float f1 = *(float *)(d + 0x218) - *(float *)(d + 0x214);
        float f2 = *(float *)(d + 0x214);
        f2 = f2 + (f1 * (f0 * *(float *)(d + 0x210)));
        *(float *)(d + 0xFC) = f2;
    }
    return;

L946C:
    func_001F9938(d + 0x20C);
    {
        float f0 = func_001FA888(*(short *)(d + 0x20C));
        float f1 = *(float *)(d + 0x214) - 0.004999999888241291f;
        f1 = f1 * (f0 * *(float *)(d + 0x210));
        *(float *)(d + 0xFC) = f1 + 0.004999999888241291f;
    }
}
