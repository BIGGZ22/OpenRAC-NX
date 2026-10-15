/* NON_MATCHING func_L10_002F72D8 -- src/overlays/shared/vendor_00299AF0.c
 * Best so far: SIZE ours 1752 / retail 1756, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera 19 update (levels 10/13/14/15): state machine on the short at f+0x14 (0 idle, 1 blend, 2 aim with a 0.1
 *   Difference: the callee-saved set and moby's register differ (retail moby $s6, saves $s0-$s7 and $fp; ours moby
 */
extern char *D_L10_0015F050 MACRO_ADDR;
extern float D_L10_0015F4FC MACRO_ADDR;
extern char *D_L10_00160058 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern char *D_L10_001B0C30[];
extern int D_L10_0015F504;
extern float D_L10_0016CFF0;
extern float D_L10_001673D8;
extern char D_L10_0016CF40[];
extern float D_L10_001674D0[] MACRO_ADDR;
extern void func_001F9EC0(void *, void *, void *);
extern void func_001F9BD8_99AF0(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9CA0(void *, void *, void *);
extern float func_001F9FA8(float);
extern float func_001F9F90(float);
extern void func_001F3140(void);
extern void func_L00_00222B80(int, int);
extern int func_L00_0025E860(void *, void *, int *, float *, float, int);
extern int func_001F9938(void *);
extern int func_001FA898(float);
extern int func_001F9850(int);
extern float func_00214220(float, float, float);
extern float func_001F9D10(void *, void *);
extern void func_001E9768(void *, void *);

/* Camera 19 update: advances the blend state and aims the camera each frame. */
void func_L10_002F72D8(char *moby) {
    float va[4];
    float v1[4];
    float v2[4];
    float v3[4];
    int ia;
    float ib;
    char *pm;
    char *dat;
    char *f;
    char *k;
    char *e;
    char *g;
    char *h;
    char *q;
    char *a;
    char *b;
    char *gb;
    char *b17;
    float *c3;
    float *kv;
    short *kk;
    short st;
    short s16;
    float t;
    float nv;
    float d;
    float sum;
    float s0;
    float s1;
    float fv;
    float w;
    float r;
    float m;
    float aa;
    float bb;
    float x;
    int rr;
    int go;
    int i;
    int F;
    int n;
    int v;

    dat = *(char **)(moby + 0x70);
    k = dat + 0x58;
    f = dat + 0x40;
    e = *(char **)(D_L10_0015F050 + *(short *)(moby + 0x84) * 0x20 + 0x1C);
    pm = moby + 0x30;
    st = *(short *)(f + 0x14);
    go = 0;

    if (st == 0) {
        t = D_L10_0015F4FC + 0.1f;
        D_L10_0015F4FC = t;
        if (1.0f <= t) {
            a = D_L10_001B0C30[*(int *)(e + 0x20)];
            b = D_L10_001B0C30[*(int *)(e + 0x24)];
            D_L10_0015F4FC = 1.0f;
            a = a + 0x10;
            D_L10_0015F504 = 1;
            *(short *)(f + 0x14) = 1;
            qcopy(pm, a);
            gb = D_L10_00160058;
            if (*(int *)(e + 0x28) >= 0) {
                gb = gb + (*(int *)(e + 0x28) << 8);
                c3 = (float *)(e + 0x2C);
                v1[0] = c3[0];
                v1[1] = c3[1];
                v1[2] = c3[2];
                v1[3] = 0.0f;
                func_001F9EC0(v1, v1, D_L10_001674D0);
                func_001F9BD8_99AF0(v2, gb + 0x10, v1);
                func_001F9BF0(va, v2, pm);
            } else {
                func_001F9BF0(va, b + 0x10, pm);
            }
            func_001F9C30(v1, D_0013E633 + 0x10AD, -1.0f);
            func_L00_001FF4B0(moby, va, 1.0f);
            func_001F9CA0(moby + 0x10, moby, v1);
            func_L00_001FF4B0(moby + 0x10, moby + 0x10, 1.0f);
            func_001F9CA0(moby + 0x20, moby + 0x10, moby);
        }
    } else if (st == 2) {
        t = D_L10_0015F4FC + 0.1f;
        D_L10_0015F4FC = t;
        if (1.0f <= t) {
            *(short *)(f + 0x14) = 3;
            D_L10_0015F4FC = 1.0f;
            x = *(float *)(k + 0x30) * 0.5f;
            aa = func_001F9FA8(x);
            bb = func_001F9F90(x);
            D_L10_0016CFF0 = aa / bb;
            func_001F3140();
            *(unsigned char *)(e + 0x39) = 1;
            D_L10_001673D8 = 0.1f;
            *(short *)(moby + 0x7E) = 4;
            D_L10_0015F504 = 0;
            if (*(int *)(e + 0x4C) == 0) {
                func_L00_00222B80(0, 1);
            }
        }
        if (*(int *)(e + 0x40) != 0) {
            go = 1;
        }
    } else if (st != 3) {
        t = D_L10_0015F4FC - 0.1f;
        D_L10_0015F4FC = t;
        if (t < 0.0f) {
            D_L10_0015F4FC = 0.0f;
        }
        go = 1;
    }

    if (go) {
        g = D_L10_001B0C30[*(int *)(e + 0x20)];
        qcopy(va, pm);
        rr = func_L00_0025E860(g, pm, (int *)f, (float *)(f + 4), *(float *)(e + 0x3C) * D_0015EE6C, 0);
        if (*(short *)(f + 0x16) != 0) {
            if (func_001F9938(f + 0x16) != 0) {
                rr = 1;
            } else if (*(int *)(e + 0x40) != 0) {
                s16 = *(short *)(f + 0x16);
                v = func_001F9850(func_001FA898(20.0f));
                if (s16 < v) {
                    rr = 1;
                }
            }
        }
        if (*(unsigned char *)(e + 0x39) != 0) {
            rr = 1;
        }
        if (rr != 0) {
            *(short *)(f + 0x14) = 2;
            if (*(int *)(e + 0x40) == 0) {
                return;
            }
        }
        kv = (float *)(k + 0x10);
        kk = (short *)k;
        if (kk[0] >= 0 && kk[1] >= 0) {
            i = 0;
            for (;;) {
                F = *(int *)f;
                if (F >= kk[i] && F < kk[i + 1]) {
                    sum = 0.0f;
                    fv = *(float *)(f + 4);
                    if (kk[i] < F) {
                        n = F - kk[i];
                        q = g + kk[i] * 16 + 0x1C;
                        do {
                            w = *(float *)q;
                            n = n - 1;
                            q = q + 16;
                            sum = sum + w;
                        } while (n != 0);
                    }
                    s1 = sum;
                    s0 = sum + fv;
                    if (F < kk[i + 1]) {
                        n = kk[i + 1] - F;
                        q = g + F * 16 + 0x1C;
                        do {
                            w = *(float *)q;
                            n = n - 1;
                            q = q + 16;
                            s1 = s1 + w;
                        } while (n != 0);
                    }
                    r = func_00214220(kv[i], kv[i + 1], s0 / s1);
                    m = r * 0.017453292f * 0.5f;
                    aa = func_001F9FA8(m);
                    bb = func_001F9F90(m);
                    *(float *)(D_L10_0016CF40 + 0xB0) = aa / bb;
                    func_001F3140();
                }
                if (!(i + 1 < 7 && kk[i + 1] >= 0 && kk[i + 2] >= 0)) {
                    break;
                }
                i = i + 1;
            }
        }
        d = func_001F9D10(va, pm);
        nv = *(float *)(f + 0x10) + d;
        *(float *)(f + 0x10) = nv;
        if (*(int *)(e + 0x28) < 0) {
            h = D_L10_001B0C30[*(int *)(e + 0x24)];
            ia = 0;
            ib = 0.0f;
            func_L00_0025E860(h, v2, &ia, &ib, (nv / *(float *)(f + 8)) * *(float *)(f + 0xC), 0);
            ((void (*)(void *, float))func_001E9768)(v2, 0.2f);
            func_001E9768(h + 0x10, h + (*(int *)h << 4));
            func_001F9BF0(v1, v2, pm);
        } else {
            b17 = D_L10_00160058 + (*(int *)(e + 0x28) << 8);
            c3 = (float *)(e + 0x2C);
            v2[0] = c3[0];
            v2[1] = c3[1];
            v2[2] = c3[2];
            v2[3] = 0.0f;
            func_001F9EC0(v2, v2, D_L10_001674D0);
            func_001F9BD8_99AF0(v3, b17 + 0x10, v2);
            func_001F9BF0(v1, v3, pm);
        }
        func_001F9C30(v2, D_0013E633 + 0x10AD, -1.0f);
        func_L00_001FF4B0(moby, v1, 1.0f);
        func_001F9CA0(moby + 0x10, moby, v2);
        func_L00_001FF4B0(moby + 0x10, moby + 0x10, 1.0f);
        func_001F9CA0(moby + 0x20, moby + 0x10, moby);
    }
}
