/* functional: high. corrected from nonmatching/functional/func_L00_00255B90.c: the four normalisations took 1/sqrtf(0) = inf when a blended quaternion is all zero, giving 0*inf = NaN; the VU0 rsqrt gives the largest float there, so the zero quaternion stays zero (rsq_255B90). Checked against the assembly lane by lane otherwise (DMA layout, lists A-D, blends, both record chains): same reads, writes, branches and arithmetic order. Builds the joint matrices of an animated moby in the scratchpad: DMAs the bind data (record at a0->0x24) and two animation tables (a0->0x68 / 0x6C), writes per-joint quaternion (slot+0), scale (+0x10), translation vectors (+0x20/+0x30), blends the two tables by a0->0x54 (lerp, or normalised lerp when the frames differ), then applies the record chains at a0->0x60 / 0x64 (quaternion products and blends).
 * Each DMA transfer to the scratchpad is a copy made where retail starts it (spr_dma_255B90), which
 * leaves the channel idle: the port has no DMA controller, and the waits for the transfers spun forever
 * (the bomb glove froze the game). */
extern float sqrtf(float);

#define ST4(p, a, b, c, d) do { float *_p = (float *)(p); _p[0] = (a); _p[1] = (b); _p[2] = (c); _p[3] = (d); } while (0)
#define LD4(v, p) do { float *_p = (float *)(p); v[0] = _p[0]; v[1] = _p[1]; v[2] = _p[2]; v[3] = _p[3]; } while (0)
#define PUT4(p, v) ST4(p, v[0], v[1], v[2], v[3])
#define UC(e, i) ((float)(((int)(((e) >> (16 * (i))) & 0xFFFF)) << 3) / 32768.0f)
#define SC0(e, i) ((float)(short)((e) >> (16 * (i))))
#define NEG(d) ((*(unsigned int *)&(d)) >> 31)
#define DOT4(v) ((((v)[1] + (v)[0]) + (v)[2]) + (v)[3])
/* A transfer of QWC quadwords from MADR to the scratchpad at SADR, done at once. */
static void spr_dma_255B90(unsigned int sadr, unsigned int madr, int qwc)
{
    int *dst = (int *)(0x70000000 + (sadr & 0x3FF0));
    int *src = (int *)madr;
    int i;

    for (i = 0; i < qwc * 4; i++)
        dst[i] = src[i];
    *(volatile unsigned int *)0x1000D400 = 0;
}

/* vrsqrt Q, vf0w, ll: 1/sqrt(ll), and the largest float when ll is zero (the VU never gives
   infinity), so a zero quaternion times Q stays zero instead of becoming NaN. ll is a sum of
   squares, never negative. */
static float rsq_255B90(float ll)
{
    union { unsigned int u; float f; } mx;

    if (!(ll > 0.0f)) {
        mx.u = 0x7F7FFFFF;
        return mx.f;
    }
    return 1.0f / sqrtf(ll);
}

#define LSQ4(v) ((((v)[0] * (v)[0] + (v)[1] * (v)[1]) + (v)[2] * (v)[2]) + (v)[3] * (v)[3])

void func_L00_00255B90(char *s)
{
    char *sp = (char *)0x70000000;
    char *rec = *(char **)(s + 0x24);
    int p1 = *(int *)(s + 0x68);
    int p2 = *(int *)(s + 0x6C);
    char *t1 = (char *)p1;
    char *t2 = (char *)p2;
    int n = *(unsigned char *)(rec + 8);
    int tmask = *(int *)(s + 0x54);
    int q1, cA, cB, cC, cD;
    char *a, *b, *c, *d, *e, *lst, *lend;
    char *r, *rr;
    int k, m, i;
    float x4, w4, Q, dd, ll;
    float f5[4], f6[4], f7[4], f8[4], f9[4], f10[4], f11[4], f12[4], f14[4], f16[4];
    unsigned long ent;
    int flag, slotv;
    char *slot;

    /* DMA 1: bind records into the scratchpad at 0x2000 */
    *(long *)(sp + n * 16 + 0x1FF8) = 0;
    spr_dma_255B90(0x2000, *(int *)(rec + 0x18), n);

    q1 = *(short *)(t1 + 6);
    cA = *(short *)(t1 + 0xA);
    cB = *(short *)(t1 + 0xE);
    ST4(sp + q1 * 16 + 0x2FF0, 0, 0, 0, 0);

    /* slot +0 and +0x10 start as (1,1,1,0) */
    k = n;
    a = sp - 0x80;
    do {
        ST4(a + 0x80, 1.0f, 1.0f, 1.0f, 0.0f);
        ST4(a + 0x90, 1.0f, 1.0f, 1.0f, 0.0f);
        a += 0x80;
        ST4(a + 0x40, 1.0f, 1.0f, 1.0f, 0.0f);
        ST4(a + 0x50, 1.0f, 1.0f, 1.0f, 0.0f);
        k -= 2;
    } while (k > 0);


    /* DMA 2: table 1 into 0x3000 */
    spr_dma_255B90(0x3000, p1 + 0x10, q1);

    /* copy the bind records to slot +0x20 (vector) and +0x30 (xyz, 0) */
    k = n;
    b = sp + 0x2000;
    c = sp - 0x100;
    do {
        c += 0x100;
        for (i = 0; i < 4; i++) {
            float *q = (float *)(b + 0x10 * i);
            ST4(c + 0x20 + 0x40 * i, q[0], q[1], q[2], q[3]);
        }
        for (i = 0; i < 4; i++) {
            float *q = (float *)(b + 0x10 * i);
            ST4(c + 0x30 + 0x40 * i, q[0], q[1], q[2], 0.0f);
        }
        b += 0x40;
        k -= 4;
    } while (k > 0);

    if (tmask != 0) {
        int b50 = *(unsigned char *)(s + 0x50);
        int b51 = *(unsigned char *)(s + 0x51);
        int b52 = *(unsigned char *)(s + 0x52);
        int b53 = *(unsigned char *)(s + 0x53);
        int differ;
        int q2, cC2, cD2;
        float *tp;

        differ = (b52 - b53) | (b50 + 1 - b51);
        x4 = *(float *)&tmask;
        w4 = 1.0f - x4;
        q2 = *(short *)(t2 + 6);
        cC = *(short *)(t2 + 0xA);
        cD = *(short *)(t2 + 0xE);
        ST4(sp + q2 * 16 + 0x35F0, 0, 0, 0, 0);

        /* DMA 3: table 2 into 0x3600 */
        spr_dma_255B90(0x3600, p2 + 0x10, q2);

        c = sp + 0x3D00; /* list of second-table slots */
        b = sp + 0x3C00; /* list of first-table slots */
        a = sp + 0x3000 + n * 8;

        /* list A: unsigned fixed point vectors into slot+0 */
        ent = *(unsigned long *)a;
        if (cA != 0) {
            while (cA > 0) {
                flag = (int)(ent >> 62);
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = UC(ent, 0); f5[1] = UC(ent, 1); f5[2] = UC(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                cA--;
                if (flag == 0)
                    continue;
                *(int *)b = (int)slot;
                b += 4;
                ST4(slot, f5[0], f5[1], f5[2], 0.0f);
                *(int *)(slot + 0xC) = -4;
            }
        }
        /* list B: signed vectors into slot+0x20 (w word kept) */
        if (cB != 0) {
            do {
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                *(int *)c = (int)slot;
                c += 4;
                slotv = *(int *)(slot + 0x2C);
                ST4(slot + 0x20, f5[0], f5[1], f5[2], 1.0f);
                cB--;
                *(int *)(slot + 0x2C) = slotv;
                *(int *)(slot + 0x3C) = -4;
            } while (cB > 0);
        }

        a = sp + 0x3600 + n * 8;

        /* list C: unsigned vectors into slot+0x10 */
        ent = *(unsigned long *)a;
        if (cC != 0) {
            while (cC > 0) {
                flag = (int)(ent >> 62);
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = UC(ent, 0); f5[1] = UC(ent, 1); f5[2] = UC(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                *(int *)b = (int)slot;
                slotv = *(int *)(slot + 0xC);
                cC--;
                if (flag == 0)
                    continue;
                ST4(slot + 0x10, f5[0], f5[1], f5[2], 0.0f);
                b += 4;
                b += slotv;
                if (cC == 0)
                    break;
            }
        }
        /* list D: signed vectors into slot+0x30 */
        if (cD != 0) {
            do {
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                *(int *)c = (int)slot;
                slotv = *(int *)(slot + 0x3C);
                c += 4;
                ST4(slot + 0x30, f5[0], f5[1], f5[2], 1.0f);
                cD--;
                c += slotv;
            } while (cD > 0);
        }

        /* blend slot+0 with slot+0x10 into slot+0x10 */
        lst = sp + 0x3C00;
        d = *(char **)lst;
        *(int *)b = (int)(sp + 0x2000);
        if (lst != b) {
            LD4(f5, d);
            LD4(f6, d + 0x10);
            do {
                f7[0] = f5[0] * w4 + f6[0] * x4;
                f7[1] = f5[1] * w4 + f6[1] * x4;
                f7[2] = f5[2] * w4 + f6[2] * x4;
                f7[3] = *(float *)(d + 0x1C);
                slotv = *(int *)(d + 0x1C);
                e = d;
                d = *(char **)(lst + 4);
                lst += 4;
                LD4(f5, d);
                LD4(f6, d + 0x10);
                ST4(e + 0x10, f7[0], f7[1], f7[2], f7[3]);
                *(int *)(e + 0x1C) = slotv;
            } while (lst != b);
        }
        /* blend slot+0x20 with slot+0x30 into slot+0x20 */
        lst = sp + 0x3D00;
        d = *(char **)lst;
        *(int *)c = (int)(sp + 0x2000);
        if (lst != c) {
            LD4(f5, d + 0x20);
            LD4(f6, d + 0x30);
            do {
                f7[0] = f5[0] * w4 + f6[0] * x4;
                f7[1] = f5[1] * w4 + f6[1] * x4;
                f7[2] = f5[2] * w4 + f6[2] * x4;
                f7[3] = *(float *)(d + 0x2C);
                slotv = *(int *)(d + 0x2C);
                e = d;
                d = *(char **)(lst + 4);
                lst += 4;
                LD4(f5, d + 0x20);
                LD4(f6, d + 0x30);
                ST4(e + 0x20, f7[0], f7[1], f7[2], f7[3]);
                *(int *)(e + 0x2C) = slotv;
            } while (lst != c);
        }

        /* rotations: lerp of the two tables, normalised when the frames differ */
        k = n;
        m = 0;
        do {
            short *h = (short *)(sp + 0x3000 + m * 16);
            short *g = (short *)(sp + 0x3600 + m * 16);
            for (i = 0; i < 4; i++) {
                f5[i] = (float)h[i] / 32768.0f;
                f6[i] = (float)h[4 + i] / 32768.0f;
                f7[i] = (float)g[i] / 32768.0f;
                f8[i] = (float)g[4 + i] / 32768.0f;
            }
            if (differ == 0) {
                for (i = 0; i < 4; i++) {
                    f9[i] = f5[i] * w4 + f7[i] * x4;
                    f10[i] = f6[i] * w4 + f8[i] * x4;
                }
                PUT4(sp + m * 0x80, f9);
                PUT4(sp + m * 0x80 + 0x40, f10);
            } else {
                for (i = 0; i < 4; i++) {
                    f5[i] = f5[i] * w4;
                    f7[i] = f7[i] * x4;
                    f6[i] = f6[i] * w4;
                    f8[i] = f8[i] * x4;
                    f9[i] = f5[i] + f7[i];
                    f10[i] = f5[i] * f7[i];
                    f11[i] = f6[i] + f8[i];
                    f12[i] = f6[i] * f8[i];
                }
                dd = DOT4(f10);
                if (NEG(dd))
                    for (i = 0; i < 4; i++)
                        f9[i] = f5[i] - f7[i];
                ll = LSQ4(f9);
                Q = rsq_255B90(ll);
                for (i = 0; i < 4; i++)
                    f9[i] = f9[i] * Q;
                dd = DOT4(f12);
                if (NEG(dd))
                    for (i = 0; i < 4; i++)
                        f11[i] = f6[i] - f8[i];
                ll = LSQ4(f11);
                Q = rsq_255B90(ll);
                for (i = 0; i < 4; i++)
                    f11[i] = f11[i] * Q;
                PUT4(sp + m * 0x80, f9);
                PUT4(sp + m * 0x80 + 0x40, f11);
            }
            m++;
            k -= 2;
        } while (k > 0);
    } else {
        /* no blend: table 1 only */
        char *w;

        a = sp + 0x3000 + n * 8;
        ent = *(unsigned long *)a;
        /* list A: unsigned vectors into slot+0x10 */
        if (cA != 0) {
            while (cA > 0) {
                flag = (int)(ent >> 62);
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = UC(ent, 0); f5[1] = UC(ent, 1); f5[2] = UC(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                cA--;
                if (flag == 0)
                    continue;
                ST4(slot + 0x10, f5[0], f5[1], f5[2], 0.0f);
            }
        }
        /* list B: signed vectors into slot+0x20 (w word kept) */
        if (cB != 0) {
            do {
                slot = sp + (*(unsigned char *)(a + 6) << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                slotv = *(int *)(slot + 0x2C);
                ST4(slot + 0x20, f5[0], f5[1], f5[2], 1.0f);
                cB--;
                *(int *)(slot + 0x2C) = slotv;
            } while (cB > 0);
        }
        /* rotations straight from table 1, four joints per pass */
        k = n;
        w = sp - 0x100;
        a = sp + 0x3000;
        do {
            short *h = (short *)a;
            short *g = (short *)(a + 0x10);
            w += 0x100;
            for (i = 0; i < 4; i++) {
                f5[i] = (float)h[i] / 32768.0f;
                f6[i] = (float)h[4 + i] / 32768.0f;
                f7[i] = (float)g[i] / 32768.0f;
                f8[i] = (float)g[4 + i] / 32768.0f;
            }
            a += 0x20;
            PUT4(w, f5);
            PUT4(w + 0x40, f6);
            PUT4(w + 0x80, f7);
            PUT4(w + 0xC0, f8);
            k -= 4;
        } while (k > 0);
    }

    /* record chain 1 (a0->0x60): DMA each batch of 3-quad records and blend into slots */
    r = *(char **)(s + 0x60);
    rr = *(char **)(s + 0x64);
    while (r != 0) {
        unsigned int w3 = *(unsigned short *)(r + 2);
        char *recs, *dst, *nxt, *nrec, *end;
        int rw3, rec1c;
        float v6[4], v7[4], v8[4], v9[4], v10[4], v11[4], v12[4], v14[4];
        int tbits;

        spr_dma_255B90(0x2000, *(int *)(r + 0x10), w3 * 3);
        end = sp + 0x2000 + ((w3 * 3) << 4);
        tbits = *(int *)(r + 8);
        x4 = *(float *)&tbits;
        recs = sp + 0x2000;
        nrec = *(char **)(r + 0x1C);
        *(int *)(end + 0x2C) = (int)(sp + 0x2000);
        w4 = 1.0f - x4;
        dst = *(char **)(recs + 0x2C);
        LD4(v9, recs);
        LD4(v10, recs + 0x10);
        LD4(v11, recs + 0x20);
        LD4(v6, dst);
        LD4(v7, dst + 0x10);
        LD4(v8, dst + 0x20);
        do {
            for (i = 0; i < 4; i++) {
                v6[i] = v6[i] * w4;
                v9[i] = v9[i] * x4;
            }
            for (i = 0; i < 3; i++) {
                v7[i] = v7[i] * w4;
                v10[i] = v10[i] * x4;
                v8[i] = v8[i] * w4;
                v11[i] = v11[i] * x4;
            }
            for (i = 0; i < 4; i++) {
                v12[i] = v6[i] + v9[i];
                v14[i] = v6[i] * v9[i];
            }
            for (i = 0; i < 3; i++) {
                v7[i] = v7[i] + v10[i];
                v8[i] = v8[i] + v11[i];
            }
            dd = DOT4(v14);
            if (NEG(dd))
                for (i = 0; i < 4; i++)
                    v12[i] = v6[i] - v9[i];
            ll = LSQ4(v12);
            Q = rsq_255B90(ll);
            rw3 = *(int *)(recs + 0x1C);
            nxt = *(char **)(recs + 0x5C);
            LD4(v9, recs + 0x30);
            LD4(v10, recs + 0x40);
            LD4(v11, recs + 0x50);
            PUT4(dst + 0x10, v7);
            PUT4(dst + 0x20, v8);
            LD4(v6, nxt);
            LD4(v7, nxt + 0x10);
            LD4(v8, nxt + 0x20);
            rec1c = *(int *)(dst + 0x1C);
            if (rec1c == 0)
                *(int *)(dst + 0x1C) = rw3;
            for (i = 0; i < 4; i++)
                v12[i] = v12[i] * Q;
            recs += 0x30;
            PUT4(dst, v12);
            dst = nxt;
        } while (recs != end);
        r = nrec;
    }

    /* record chain 2 (a0->0x64): combine rotation / scale / translation of a slot with a record */
    while (rr != 0) {
        float v6[4], v7[4], v8[4], v9[4], v10[4], v11[4], v12[4], v13[4], v14[4], v15[4], v16[4];
        char *tg;
        int sel;
        float ax, ay, az;

        r = rr;
        tg = *(char **)(r + 4);
        sel = *(unsigned char *)(r + 3);
        LD4(v6, tg);
        LD4(v8, tg + 0x10);
        LD4(v10, tg + 0x20);
        LD4(v7, r + 0x10);
        LD4(v9, r + 0x20);
        LD4(v11, r + 0x30);
        rr = *(char **)(r + 8);
        if (sel == 0) {
            v12[3] = v7[3] * v6[3];
            for (i = 0; i < 3; i++) {
                v13[i] = v7[i] * v6[i];
                v14[i] = v7[i] * v6[3];
                v15[i] = v6[i] * v7[3];
            }
            v16[0] = v6[1] * v7[2] - v7[1] * v6[2];
            v16[1] = v6[2] * v7[0] - v7[2] * v6[0];
            v16[2] = v6[0] * v7[1] - v7[0] * v6[1];
            ax = (v13[0] + v13[1]) + v13[2];
            for (i = 0; i < 3; i++) {
                v6[i] = v14[i] + v15[i];
                v6[i] = v6[i] + v16[i];
            }
            v6[3] = v12[3] - ax;
            for (i = 0; i < 3; i++) {
                v8[i] = v8[i] * v9[i];
                v10[i] = v10[i] + v11[i];
            }
        } else {
            int bb = *(int *)(r + 0xC);
            float x18 = *(float *)&bb;
            float w18 = 1.0f - x18;
            for (i = 0; i < 4; i++) {
                v6[i] = v6[i] * w18;
                v7[i] = v7[i] * x18;
            }
            for (i = 0; i < 3; i++) {
                v8[i] = v8[i] * w18;
                v9[i] = v9[i] * x18;
                v10[i] = v10[i] * w18;
                v11[i] = v11[i] * x18;
            }
            for (i = 0; i < 4; i++) {
                v14[i] = v6[i] + v7[i];
                v16[i] = v6[i] * v7[i];
            }
            for (i = 0; i < 3; i++) {
                v8[i] = v8[i] + v9[i];
                v10[i] = v10[i] + v11[i];
            }
            dd = DOT4(v16);
            if (NEG(dd))
                for (i = 0; i < 4; i++)
                    v14[i] = v6[i] - v7[i];
            ll = LSQ4(v14);
            Q = rsq_255B90(ll);
            for (i = 0; i < 4; i++)
                v6[i] = v14[i] * Q;
        }
        PUT4(tg, v6);
        PUT4(tg + 0x10, v8);
        PUT4(tg + 0x20, v10);
    }
}
