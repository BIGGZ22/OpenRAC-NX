/* NON_MATCHING func_L00_002D2E60 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 1244 / retail 1228, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Stopped after 5 runs (size wrong: 1244 vs 1228). Shared overlay routine: builds a 102-entry vector fan (two br
 *   Left: our gp-table reads come out as lui+lw (absolute) where retail uses $gp, which costs 4 bytes each; the 64
 */
typedef int u128 __attribute__((mode(TI)));
extern int D_001139AC SDATA(D_001139AC);
extern int D_001139B0 SDATA(D_001139B0);
extern int D_001139B4 SDATA(D_001139B4);
extern int D_001139B8 SDATA(D_001139B8);
extern int D_001139BC SDATA(D_001139BC);
extern int D_001139C0 SDATA(D_001139C0);
extern int D_001139C4 SDATA(D_001139C4);
extern char D_L00_00166D80[];
extern char D_L00_00166EC0[];
extern char D_L00_001E0E50[];
extern char D_L00_001E1180[];
extern char D_L00_001E07F0[];
extern char D_L00_001DEB00[];
extern char D_L00_001DFCF0[];
extern char D_L00_001DF160[];
extern int func_001F4868(int);
extern float func_001F9B88(float);
extern int func_001F9908(int *);
extern float func_001FA888(int);
extern int func_001F9850(int);
extern void func_001F9BF0(void *, void *, void *);
extern void func_L00_001FF4B0(void *, void *, float);
extern float func_001F9C78(void *a, void *b);
extern void func_001F9C30(void *, void *, float);
extern float func_001F9B50(float);
extern void func_L00_001FD1D8(void *, void *, int);

/* Builds a 102-entry vector fan from the level tables, then a 74-record index walk that copies entries out into a GS packet buffer. */
void func_L00_002D2E60(char *moby) {
    char *d;
    int flag = 0;
    int i;
    int k;
    int r;
    int c3;
    int q[4];
    float xy[8];
    float vv[16];
    u64 pk[4];
    float v90[4];
    float va0[4];
    float vb0[4];
    char *p6;
    char *p7;
    char *p8;
    char *out;
    char *a16;
    char *b5;
    char *s8;
    char *vp;
    char *p16;
    char *p17;
    char *p19;
    char *p20;
    char *p21;
    char *p22;
    char *p30;
    float f20, f21, f22, f0, f1, f2, f3, f4, dot, t, o0, o1;
    long a, b, c, dd, e;
    u64 packed;
    int idx;

    d = *(char **)(moby + 0x78);
    r = func_001F4868(D_001139C0);
    f20 = 16.0f;
    a = D_001139AC;
    b = D_001139B0;
    c = D_001139B4;
    dd = D_001139B8;
    e = D_001139BC;
    packed = (u64)(a | (b << 2) | (c << 4) | (dd << 6) | (e << 32));
    c3 = D_001139C4;
    pk[0] = 0;
    pk[1] = (u64)(long)r;
    pk[2] = 0xFF9000000260UL;
    pk[3] = packed;
    q[0] = c3;
    q[1] = c3;
    q[2] = c3;
    q[3] = c3;

    if (func_001F9B88(*(float *)(D_L00_00166D80 + 0x140) - *(float *)(moby + 0x10)) < f20) {
        if (func_001F9B88(*(float *)(D_L00_00166D80 + 0x144) - *(float *)(moby + 0x14)) < f20) {
            flag = 1;
        }
    }

    p6 = (char *)xy;
    p7 = (char *)xy + 4;

    if (flag == 0 && *(unsigned char *)(moby + 0x20) != 1) {
        if (*(unsigned char *)(moby + 0xBC) == 1) {
            p20 = D_L00_001DEB00;
            p16 = D_L00_001E1180;
            p19 = D_L00_001E07F0;
            *(unsigned char *)(moby + 0xBC) = 0;
            p17 = D_L00_001E0E50;
            k = 0x65;
            do {
                f0 = *(float *)p17;
                f1 = *(float *)(p17 + 4);
                *(float *)p16 = f0;
                *(float *)(p16 + 4) = f1;
                func_001F9EE8((void *)p19, (void *)p20, d);
                p20 += 0x10;
                p19 += 0x10;
                p16 += 8;
                k = k - 1;
                p17 += 8;
            } while (k >= 0);
        } else {
            p16 = D_L00_001E07F0;
            p17 = D_L00_001DEB00;
            k = 0x65;
            do {
                func_001F9EE8((void *)p16, (void *)p17, d);
                p17 += 0x10;
                k = k - 1;
                p16 += 0x10;
            } while (k >= 0);
        }
        *(int *)(d + 0x40) = func_001F9850(0x3C);
    } else {
        *(unsigned char *)(moby + 0xBC) = 1;
        func_001F9908((int *)(d + 0x40));
        f22 = 1.0f;
        f21 = 0.5f;
        f20 = func_001FA888(*(int *)(d + 0x40));
        idx = func_001F9850(0x3C);
        f20 = f20 / func_001FA888(idx);

        p8 = D_L00_001E1180;
        out = D_L00_001E0E50;
        for (i = 0; i < 0x66; i++) {
            a16 = D_L00_001E07F0 + (i << 4);
            b5 = D_L00_001DEB00 + (i << 4);
            func_001F9EE8((void *)a16, (void *)b5, d);
            func_001F9BF0(vb0, a16, D_L00_00166EC0);
            func_L00_001FF4B0(vb0, vb0, f22);
            func_001F9EE8((void *)v90, (void *)(D_L00_001DFCF0 + (i << 4)), d);
            func_L00_001FF4B0(v90, v90, 0.1f);
            dot = func_001F9C78(v90, vb0);
            func_001F9C30(va0, v90, dot + dot);
            func_001F9BF0(va0, vb0, va0);
            func_L00_001FF4B0(va0, va0, f22);
            va0[2] = va0[2] + f22;
            t = func_001F9B50(va0[2] + va0[2]);
            f4 = t + t;
            if (*(unsigned char *)(moby + 0x20) == 1 || *(int *)(d + 0x40) == 0) {
                o0 = va0[0] / f4 + f21;
                o1 = va0[1] / f4 + f21;
            } else {
                f2 = va0[0] / f4 + f21;
                f3 = va0[1] / f4;
                f0 = *(float *)p8 - f2;
                f0 = f0 * f20;
                o0 = f2 + f0;
                f2 = f3 + f21;
                f1 = *(float *)(p8 + 4) - f2;
                f1 = f1 * f20;
                o1 = f2 + f1;
            }
            *(float *)out = o0;
            *(float *)(out + 4) = o1;
            p8 += 8;
            out += 8;
        }

        if (*(unsigned char *)(moby + 0x20) == 1) {
            *(unsigned char *)(moby + 0x20) = 2;
        }
    }

    p30 = D_L00_001DF160;
    p16 = D_L00_001E0E50;
    p22 = p6;
    p21 = p7;
    p20 = p16 + 4;
    p19 = D_L00_001E07F0;
    for (i = 0; i < 0x4A; i++) {
        s8 = p30 + i * 24;
        p6 = p22;
        p7 = p21;
        vp = (char *)vv;
        for (k = 3; k >= 0; k--) {
            idx = *(short *)s8;
            *(u128 *)vp = *(u128 *)(p19 + (idx << 4));
            vp += 16;
            *(float *)p6 = *(float *)(p16 + (idx << 3));
            p6 += 8;
            *(float *)p7 = *(float *)(p20 + (idx << 3));
            p7 += 8;
            s8 += 6;
        }
        func_L00_001FD1D8(vv, 0, 0);
    }
}
