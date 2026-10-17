/* functional: high. Corrected from nonmatching/shared/func_L06_002F3680.c: (1) sel/fl/ptrs were [20] but jc runs to 50 (retail frame has room for 50), so the host overran the stack; (2) candidate filter skipped when byte 0xBC != 2, retail skips when it == 2; (3) insertion index k was uninitialised when the list was empty (retail sets 0); (4) the clear loop read *(outv[i]) instead of outv[i]+0x78; (5) the selection loop ran jc-1 times, retail tests the old counter and runs jc times. */
#include "common.h"
extern char *D_L06_001AC500[];
extern char D_L06_001746C0_c[] __asm__("D_L06_001746C0");
extern int D_L06_0015F6B0 MACRO_ADDR;
extern float func_001F9D48(void *, void *);
extern float func_001F9B88(float);
extern int func_L00_001EFFF0(void *, void *, int, int, int);
extern float func_L06_002F3578(char *a, char *b, int c);
extern unsigned char *func_L00_0025D390(int);
extern float func_001FA748(float, float);

/* Shared level-06/10 routine: gathers candidate mobies, scores them against the target list and writes the slot tables. */
void func_L06_002F3680(void) {
    char *outv[50];
    char vD[16] __attribute__((aligned(16)));
    char vE[16] __attribute__((aligned(16)));
    int sel[50];
    signed char fl[50];
    char *ptrs[50];
    char *pp;
    char *q;
    char *d;
    char *pi;
    char *best6;
    char **ep;
    char **op;
    int n;
    int i;
    int j;
    int jn;
    int k;
    int h;
    int cnt;
    int bi;
    int bestv;
    int m;
    int r;
    int jc;
    int found;
    int kq;
    int i2;
    int j2;
    int selv;
    int hh;
    int mval;
    int cnt2;
    int i3;
    int t;
    int dd;
    int v;
    float f0;
    float f1;
    float f12;
    float f20;
    float f21;
    float f22;
    float f23;

    n = 0;
    pp = D_L06_001AC500[0];
    if (pp != 0) {
        ep = (char **)D_L06_001AC500;
        op = outv;
        do {
            if (*(short *)(pp + 0xA6) != 0x359) goto L_NX;
            if (((unsigned char *)pp)[0x20] != 5) goto L_NX;
            if (((unsigned char *)pp)[0xBC] == 2) goto L_NX;
            if (n == 0x32) goto L_NX;
            d = *(char **)(pp + 0x78);
            q = *(char **)(d + 0x174);
            if (q == 0) goto L_ADD;
            if (((unsigned char *)q)[0x20] == 0xFE) goto L_ADD;
            if (((unsigned char *)q)[0x20] != 0xFD) goto L_NX;
        L_ADD:
            *op = pp;
            op++;
            n++;
            ep++;
            pp = *ep;
            continue;
        L_NX:
            ep++;
            pp = *ep;
        } while (pp != 0);
    }

    if (n > 0) {
        for (i = 0; i < n; i++) {
            pi = outv[i];
            d = *(char **)(pi + 0x78);
            *(short *)(d + 0x192) = 0;
            q = D_L06_001AC500[0];
            j = 0;
            while (q != 0) {
                jn = j + 1;
                cnt = (*(char **)(q + 0x24) != 0) ? *(short *)(*(char **)(q + 0x24) + 0x46) : 0;
                if (cnt != 5) goto L_396C;
                f20 = (*(short *)(d + 0x198) != 0) ? 5.0f : 12.0f;
                f0 = func_001F9D48((void *)(pi + 0x10), (void *)(q + 0x10));
                if (f20 < f0) goto L_396C;
                f12 = *(float *)(pi + 0x18) - *(float *)(q + 0x18);
                f0 = func_001F9B88(f12);
                if (1.0f < f0) goto L_396C;
                qcopy(vE, pi + 0x10);
                qcopy(vD, q + 0x10);
                f1 = *(float *)(vE + 8) + 0.25f;
                f0 = *(float *)(vD + 8) + 0.25f;
                *(float *)(vE + 8) = f1;
                *(float *)(vD + 8) = f0;
                r = func_L00_001EFFF0(vE, vD, 2, (int)pi, 0);
                if (r != 0) {
                    if (*(char **)(D_L06_001746C0_c + 0x18) != q) goto L_396C;
                }
                f1 = func_L06_002F3578(pi, q, 5);
                h = *(short *)(d + 0x192);
                k = 0;
                if (h > 0) {
                    for (;;) {
                        f0 = *(float *)(d + 0x1C0 + k * 4);
                        if (f1 < f0) {
                            t = (*(unsigned short *)(d + 0x192) << 16) >> 16;
                            t = t - 1;
                            bi = (t < 7) ? t : 6;
                            if (bi < k) break;
                            do {
                                *(int *)(d + 0x1A4 + bi * 4) = *(int *)(d + 0x1A0 + bi * 4);
                                *(float *)(d + 0x1C0 + bi * 4 + 4) = *(float *)(d + 0x1C0 + bi * 4);
                                bi--;
                            } while (bi >= k);
                            break;
                        }
                        k++;
                        if (k >= *(short *)(d + 0x192)) break;
                    }
                }
                if (k < 8) {
                    *(int *)(d + 0x1A0 + k * 4) = (int)q;
                    *(float *)(d + 0x1C0 + k * 4) = f1;
                    hh = (unsigned short)(*(unsigned short *)(d + 0x192) + 1);
                    *(short *)(d + 0x192) = (short)hh;
                    if ((short)hh >= 9) *(short *)(d + 0x192) = 8;
                }
    L_396C:
                j = jn;
                q = D_L06_001AC500[j];
            }
        }
    }

    jc = 0;
    if (n > 0) {
        for (i = 0; i < n; i++) {
            pi = outv[i];
            d = *(char **)(pi + 0x78);
            h = *(short *)(d + 0x192);
            if (h <= 0) continue;
            k = 0;
            for (;;) {
                if (jc < 0x32) {
                    dd = 8 - k;
                    for (m = 0; m < jc; m++) {
                        if (sel[m] == *(int *)(d + 0x1A0 + k * 4)) {
                            ((unsigned char *)fl)[m] += dd;
                            goto L_3A54;
                        }
                    }
                    sel[jc] = *(int *)(d + 0x1A0 + k * 4);
                    ((unsigned char *)fl)[jc] = dd;
                    jc++;
                }
    L_3A54:
                k++;
                if (k >= *(short *)(d + 0x192)) break;
            }
        }
    }

    if (n > 0) {
        int *cp = (int *)outv;
        int cc = n;
        do {
            *(int *)(*(char **)((char *)*cp + 0x78) + 0x178) = 0;
            cp++;
            cc--;
        } while (cc != 0);
    }

    if (jc > 0) {
        f23 = 0.5235988f;
        ptrs[0] = 0;
        f22 = 1.74532925f;
        kq = 1;
        do {
            best6 = 0;
            bestv = 0;
            bi = 0;
            for (m = 0; m < jc; m++) {
                if (bestv < (int)fl[m]) {
                    bestv = fl[m];
                    bi = m;
                }
            }
            fl[bi] = -1;
            selv = sel[bi];
            cnt2 = 1;
            found = 0;
            (void)selv;
            d = (char *)func_L00_0025D390(selv);
            if (d != 0) cnt2 = *(short *)(d + 0x4);
            if (cnt2 > 0) {
                for (;;) {
                    f1 = 9999999.0f;
                    best6 = 0;
                    for (i2 = 0; i2 < n; i2++) {
                        pi = outv[i2];
                        d = *(char **)(pi + 0x78);
                        if (*(int *)(d + 0x178) != 0) continue;
                        hh = *(short *)(d + 0x192);
                        if (hh <= 0) continue;
                        for (j2 = 0; j2 < hh; j2++) {
                            if (*(int *)(d + 0x1A0 + j2 * 4) == selv) {
                                if (*(float *)(d + 0x1C0 + j2 * 4) < f1) {
                                    f1 = *(float *)(d + 0x1C0 + j2 * 4);
                                    best6 = pi;
                                }
                            }
                        }
                    }
                    if (best6 == 0) break;
                    found++;
                    *(int *)(*(char **)(best6 + 0x78) + 0x178) = selv;
                    ptrs[found - 1] = best6;
                    if (!(found < cnt2)) break;
                }
            }
            if (found > 0) {
                f0 = (float)(found - 1);
                f12 = f0 * f23;
                if (f22 < f12) f12 = 1.74532925f;
                f21 = 0.0f;
                if (!(found < 2)) f21 = f12 / f0;
                f20 = f12 * -0.5f;
                for (i3 = 0; i3 < found; i3++) {
                    d = *(char **)(ptrs[i3] + 0x78);
                    f1 = func_001FA748(f20, (float)i3 * f21);
                    *(float *)(d + 0x17C) = f1;
                }
            }
        } while (kq++ < jc);
    }

    if (n > 0) {
        int cc2 = n;
        char **cq = (char **)outv;
        mval = D_L06_0015F6B0;
        do {
            d = *(char **)(*cq + 0x78);
            v = *(int *)(d + 0x178);
            *(int *)(d + 0x18C) = mval;
            *(int *)(d + 0x174) = v;
            *(short *)(d + 0x19A) = 1;
            if (v != 0) *(short *)(d + 0x198) = 1;
            cq++;
            cc2--;
        } while (cc2 != 0);
    }
    return;
}
