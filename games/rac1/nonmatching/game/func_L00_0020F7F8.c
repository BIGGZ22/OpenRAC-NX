/* NON_MATCHING func_L00_0020F7F8 -- src/overlays/shared/help_0020CDF0.c
 * Best so far: SIZE ours 1040 / retail 1056, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Shared bookkeeping over the 16-slot ring at D_0013F450 (base is D_0013E633+0xE1D): advances the ring index and
 *   Best so far p3.c: 1040 bytes against retail 1056, not EXACT. Earlier: p0 992, p1 1008, p2 1032. The gap is the
 *   Also: the -1 compare constant is a live register in retail (sq $a2,0x90 spill); the threshold selects are bran
 */
extern char D_0013E633[];
extern short D_L00_0015F774;
extern void func_001FA480();
extern void func_00214F78(float *);
extern void func_001F9BC0(void *);
extern float func_L00_0020F750(float *a, float *b);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9C30(void *, void *, float);
extern void func_002153E8(void *, void *);

typedef struct { float x, y, z, w; } __attribute__((aligned(16))) V4_F7F8;

/* Per-frame bookkeeping for the 16-slot ring at D_0013F450: advances the slot index, scores pairs of slots and writes the normalized weights to arg+0x40. */
void func_L00_0020F7F8(char *arg) {
    V4_F7F8 vv[3];
    float fa[16];
    float *fp;
    float *p;
    float *out;
    V4_F7F8 *sp4;
    V4_F7F8 *vq;
    int cnt, n, i, j, k, m, v, t, r, s, c, a, b, ra, rb, acc, neg;
    float mx, thr;

    neg = -1;
    func_001FA480(D_0013E633 + 0xE1D + 0x12F0 + *(int *)(D_0013E633 + 0xE1D + 0x21B8) * 0x30);
    func_00214F78((float *)(D_0013E633 + 0xE1D + 0x12F0 + *(int *)(D_0013E633 + 0xE1D + 0x21B8) * 0x30));

    v = *(int *)(D_0013E633 + 0xE1D + 0x21B8);
    t = v + 1;
    r = v + 16;
    if (neg < t) {
        r = t;
    }
    r = (r >> 4) << 4;
    c = *(int *)(D_0013E633 + 0xE1D + 0x21BC) + 1;
    *(int *)(D_0013E633 + 0xE1D + 0x21BC) = c;
    *(int *)(D_0013E633 + 0xE1D + 0x21B8) = t - r;
    if (!(c < 0x11)) {
        *(int *)(D_0013E633 + 0xE1D + 0x21BC) = 0x10;
    }

    {
        V4_F7F8 *w = vv;
        for (k = 2; k >= 0; k--) {
            func_001F9BC0(w);
            w++;
        }
    }

    if (*(int *)(D_0013E633 + 0xE1D + 0x208C) != 0x16) {
        if (*(int *)(D_0013E633 + 0xE1D + 0x2084) == 2) {
            s = 5;
        } else {
            s = 0xB;
        }
    } else {
        s = 0xE;
    }
    *(int *)&D_L00_0015F774 = s;

    c = *(int *)(D_0013E633 + 0xE1D + 0x21C0);
    if (c < *(int *)&D_L00_0015F774) {
        c = c + 1;
        *(int *)(D_0013E633 + 0xE1D + 0x21C0) = c;
    } else if (*(int *)&D_L00_0015F774 < c) {
        c = c - 1;
        *(int *)(D_0013E633 + 0xE1D + 0x21C0) = c;
    }

    n = *(int *)(D_0013E633 + 0xE1D + 0x21C0);
    if (!(1 < n)) {
        n = 2;
    }
    if (*(int *)(D_0013E633 + 0xE1D + 0x21BC) < n) {
        return;
    }

    out = (float *)(arg + 0x40);
    cnt = 0;
    fp = fa;
    if (n > 0) for (i = 0; i < n; i++) {
        a = *(int *)(D_0013E633 + 0xE1D + 0x21B8) - (i - 15);
        ra = a + 15;
        if (neg < a) {
            ra = a;
        }
        ra = a - ((ra >> 4) << 4);
        for (j = 0; j < n; j++) {
            if (i != j) {
                b = *(int *)(D_0013E633 + 0xE1D + 0x21B8) - (j - 15);
                rb = b + 15;
                if (neg < b) {
                    rb = b;
                }
                rb = b - ((rb >> 4) << 4);
                fp[i] += func_L00_0020F750((float *)(D_0013E633 + 0xE1D + 0x12F0 + ra * 0x30),
                                           (float *)(D_0013E633 + 0xE1D + 0x12F0 + rb * 0x30));
            }
        }
    }

    mx = 0.0f;
    p = fa;
    for (k = n; k != 0; k--) {
        if (mx < *p) {
            mx = *p;
        }
        p++;
    }

    thr = mx * 0.2f;
    p = fa;
    for (k = n; k > 0; k--) {
        if (thr < *p) {
            cnt++;
        }
        p++;
    }

    acc = 0;
    thr = mx * 0.2f;
    for (k = 0; n > 0 && k < n; k++) {
        if (thr < fp[k] && cnt > 0 && cnt < 3) {
            continue;
        }
        acc++;
        a = *(int *)(D_0013E633 + 0xE1D + 0x21B8) - (k - 15);
        ra = a + 15;
        if (neg < a) {
            ra = a;
        }
        ra = a - ((ra >> 4) << 4);
        vq = (V4_F7F8 *)(D_0013E633 + 0xE1D + 0x12F0 + ra * 0x30);
        sp4 = vv;
        for (m = 2; m >= 0; m--) {
            func_001F9BD8(sp4, sp4, vq);
            sp4++;
            vq++;
        }
    }

    {
        V4_F7F8 *q = vv;
        for (m = 2; m >= 0; m--) {
            func_001F9C30(q, q, 1.0f / (float)acc);
            q++;
        }
    }
    func_002153E8(vv, out);
}
