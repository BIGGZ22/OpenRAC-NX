/* functional: medium. Moby skeleton pose builder (handwritten VU0): a0 = moby, a1 = joint count n, a2 = output (0: none beyond the scratchpad copy, >0: game address of a vector array, <0: skip the copy-out). DMAs the bind records (a0->0x24 record, +0x18) to scratchpad 0x3000 and the animation table a0->0x68 to 0x2000 (a0->0x6C to 0x2600 when a0->0x54 holds a blend weight), unpacks per-joint quaternion (slot+0), scale (+0x10), translation (+0x20/+0x30) with the signed/unsigned fixed point lists, blends the two tables (lerp, renormalised when the frames differ: the code the catalogue lists separately as func_L00_00254A70), applies the record chains at a0->0x60 / 0x64, then turns each quaternion into a rotation matrix, scales it, concatenates it with the parent matrix and packs the translations to the start of the scratchpad (and to a2). Plain float lane arithmetic stands in for VU0, not run against retail (hand-checked against the assembly); 1/sqrt of a zero length gives a huge finite value instead of the VU maximum. Each DMA is a copy made where retail starts it (the port has no DMA controller; the waits for the transfers are dropped); the caller's $t8/$t9 saved in the scratchpad are not kept. */
extern float sqrtf(float);

#define ST4(p, a, b, c, d) do { float *_p = (float *)(p); _p[0] = (a); _p[1] = (b); _p[2] = (c); _p[3] = (d); } while (0)
#define LD4(v, p) do { float *_p = (float *)(p); v[0] = _p[0]; v[1] = _p[1]; v[2] = _p[2]; v[3] = _p[3]; } while (0)
#define PUT4(p, v) ST4(p, v[0], v[1], v[2], v[3])
#define UC(e, i) ((float)(((int)(((e) >> (16 * (i))) & 0xFFFF)) << 3) / 32768.0f)
#define SC0(e, i) ((float)(short)((e) >> (16 * (i))))
#define NEG(d) ((*(unsigned int *)&(d)) >> 31)
#define DOT4(v) ((((v)[1] + (v)[0]) + (v)[2]) + (v)[3])
#define LSQ4(v) ((((v)[0] * (v)[0] + (v)[1] * (v)[1]) + (v)[2] * (v)[2]) + (v)[3] * (v)[3])
#define HF(h, i) ((float)(h)[i] / 32768.0f)

static float rsq_254218(float ll)
{
    if (ll > 0.0f)
        return 1.0f / sqrtf(ll);
    return 3.4028235e38f;
}

/* A transfer of QWC quadwords from MADR to the scratchpad at SADR, done at once. */
static void spr_dma_254218(unsigned int sadr, unsigned int madr, int qwc)
{
    int *dst = (int *)(0x70000000 + (sadr & 0x3FF0));
    int *src = (int *)madr;
    int i;

    for (i = 0; i < qwc * 4; i++)
        dst[i] = src[i];
    *(volatile unsigned int *)0x1000D400 = 0;
}

void func_L00_00254218(void *moby, int n, int out_)
{
    char *s = (char *)moby;
    char *sp = (char *)0x70000000;
    char *rec = *(char **)(s + 0x24);
    int p1 = *(int *)(s + 0x68);
    int p2 = *(int *)(s + 0x6C);
    char *t1 = (char *)p1;
    char *t2 = (char *)p2;
    int tmask = *(int *)(s + 0x54);
    int n8 = *(unsigned char *)(rec + 8) << 3;
    int q1, cA, cB, cC, cD;
    char *a, *b, *c, *d, *e, *lst, *lend, *slot, *w;
    char *r, *rr;
    unsigned long ent;
    int k, m, i, idx, flag, slotv;
    float x4, w4, Q, dd, ll;
    float f5[4], f6[4], f7[4], f8[4], f9[4], f10[4], f11[4], f12[4];

    *(int *)(sp + 0x3720) = out_;
    *(int *)(sp + 0x3728) = n8;

    /* DMA 1: bind records into the scratchpad at 0x3000 */
    spr_dma_254218(0x3000, *(int *)(rec + 0x18), n);

    q1 = *(short *)(t1 + 6);
    cA = *(short *)(t1 + 0xA);
    cB = *(short *)(t1 + 0xE);

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

    /* DMA 2: table 1 into 0x2000 */
    spr_dma_254218(0x2000, p1 + 0x10, q1);

    /* copy the bind records to slot +0x20 (vector) and +0x30 (xyz, 0), four joints per pass */
    k = n;
    b = sp + 0x3000;
    c = sp - 0x100;
    do {
        c += 0x100;
        for (i = 0; i < 4; i++) {
            float *q = (float *)(b + 0x10 * i);
            ST4(c + 0x20 + 0x40 * i, q[0], q[1], q[2], q[3]);
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
        int q2;
        char *lb, *lc, *ld;
        char *l1, *l2;

        differ = (b52 - b53) | (b50 + 1 - b51);
        x4 = *(float *)&tmask;
        w4 = 1.0f - x4;
        q2 = *(short *)(t2 + 6);
        cC = *(short *)(t2 + 0xA);
        cD = *(short *)(t2 + 0xE);
        *(int *)(sp + 0x3724) = differ;

        /* DMA 3: table 2 into 0x2600 */
        spr_dma_254218(0x2600, p2 + 0x10, q2);

        l1 = sp + 0x2C00; /* slots whose first vector is set (written for list A and C) */
        l2 = sp + 0x2D00; /* slots whose second vector is set (list B and D) */
        a = sp + 0x2000 + n8;
        lb = a + cA * 8;

        /* list A: unsigned fixed point vectors into slot+0, marker -4 in its w word */
        if (cA != 0) {
            ent = *(unsigned long *)a;
            while (cA > 0) {
                idx = *(unsigned char *)(a + 6);
                if (n - idx <= 0)
                    break;
                flag = (int)(ent >> 62);
                slot = sp + (idx << 6);
                f5[0] = UC(ent, 0); f5[1] = UC(ent, 1); f5[2] = UC(ent, 2);
                ent = *(unsigned long *)(a + 8);
                a += 8;
                cA--;
                if (flag == 0)
                    continue;
                *(int *)l1 = (int)slot;
                l1 += 4;
                ST4(slot, f5[0], f5[1], f5[2], 0.0f);
                *(int *)(slot + 0xC) = -4;
            }
        }
        /* list B: signed vectors into slot+0x20 (w word kept), marker -4 at slot+0x3C */
        if (cB != 0) {
            ent = *(unsigned long *)lb;
            do {
                idx = *(unsigned char *)(lb + 6);
                if (n - idx <= 0)
                    break;
                slot = sp + (idx << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(lb + 8);
                lb += 8;
                *(int *)l2 = (int)slot;
                l2 += 4;
                slotv = *(int *)(slot + 0x2C);
                ST4(slot + 0x20, f5[0], f5[1], f5[2], 1.0f);
                cB--;
                *(int *)(slot + 0x2C) = slotv;
                *(int *)(slot + 0x3C) = -4;
            } while (cB > 0);
        }

        lc = sp + 0x2600 + n8;
        ld = lc + cC * 8;

        /* list C: unsigned vectors into slot+0x10; a slot already listed pulls the list back */
        if (cC != 0) {
            ent = *(unsigned long *)lc;
            while (cC > 0) {
                idx = *(unsigned char *)(lc + 6);
                if (n - idx <= 0)
                    break;
                flag = (int)(ent >> 62);
                slot = sp + (idx << 6);
                f5[0] = UC(ent, 0); f5[1] = UC(ent, 1); f5[2] = UC(ent, 2);
                ent = *(unsigned long *)(lc + 8);
                lc += 8;
                *(int *)l1 = (int)slot;
                slotv = *(int *)(slot + 0xC);
                cC--;
                if (flag == 0)
                    continue;
                ST4(slot + 0x10, f5[0], f5[1], f5[2], 0.0f);
                l1 += 4;
                l1 += slotv;
            }
        }
        /* list D: signed vectors into slot+0x30 */
        if (cD != 0) {
            ent = *(unsigned long *)ld;
            do {
                idx = *(unsigned char *)(ld + 6);
                if (n - idx <= 0)
                    break;
                slot = sp + (idx << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(ld + 8);
                ld += 8;
                *(int *)l2 = (int)slot;
                slotv = *(int *)(slot + 0x3C);
                l2 += 4;
                ST4(slot + 0x30, f5[0], f5[1], f5[2], 1.0f);
                cD--;
                l2 += slotv;
            } while (cD > 0);
        }

        /* blend slot+0 with slot+0x10 into slot+0x10 */
        lst = sp + 0x2C00;
        d = *(char **)lst;
        *(int *)l1 = (int)(sp + 0x1E00);
        if (lst != l1) {
            LD4(f5, d);
            LD4(f6, d + 0x10);
            do {
                f7[0] = f5[0] * w4 + f6[0] * x4;
                f7[1] = f5[1] * w4 + f6[1] * x4;
                f7[2] = f5[2] * w4 + f6[2] * x4;
                slotv = *(int *)(d + 0x1C);
                e = d;
                d = *(char **)(lst + 4);
                lst += 4;
                LD4(f5, d);
                LD4(f6, d + 0x10);
                ST4(e + 0x10, f7[0], f7[1], f7[2], 0.0f);
                *(int *)(e + 0x1C) = slotv;
            } while (lst != l1);
        }
        /* blend slot+0x20 with slot+0x30 into slot+0x20 */
        lst = sp + 0x2D00;
        d = *(char **)lst;
        *(int *)l2 = (int)(sp + 0x1E00);
        if (lst != l2) {
            LD4(f5, d + 0x20);
            LD4(f6, d + 0x30);
            do {
                f7[0] = f5[0] * w4 + f6[0] * x4;
                f7[1] = f5[1] * w4 + f6[1] * x4;
                f7[2] = f5[2] * w4 + f6[2] * x4;
                slotv = *(int *)(d + 0x2C);
                e = d;
                d = *(char **)(lst + 4);
                lst += 4;
                LD4(f5, d + 0x20);
                LD4(f6, d + 0x30);
                ST4(e + 0x20, f7[0], f7[1], f7[2], 0.0f);
                *(int *)(e + 0x2C) = slotv;
            } while (lst != l2);
        }

        /* rotations: lerp of the two tables, normalised when the frames differ */
        k = n;
        m = 0;
        do {
            short *h = (short *)(sp + 0x2000 + m * 16);
            short *g = (short *)(sp + 0x2600 + m * 16);
            for (i = 0; i < 4; i++) {
                f5[i] = HF(h, i);
                f6[i] = HF(h, 4 + i);
                f7[i] = HF(g, i);
                f8[i] = HF(g, 4 + i);
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
                Q = rsq_254218(LSQ4(f9));
                for (i = 0; i < 4; i++)
                    f9[i] = f9[i] * Q;
                dd = DOT4(f12);
                if (NEG(dd))
                    for (i = 0; i < 4; i++)
                        f11[i] = f6[i] - f8[i];
                Q = rsq_254218(LSQ4(f11));
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
        a = sp + 0x2000 + n8;
        b = a + cA * 8;

        /* list A: unsigned vectors into slot+0x10 (w 0) */
        ent = *(unsigned long *)a;
        if (cA != 0) {
            while (cA > 0) {
                idx = *(unsigned char *)(a + 6);
                if (n - idx <= 0)
                    break;
                flag = (int)(ent >> 62);
                slot = sp + (idx << 6);
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
            ent = *(unsigned long *)b;
            do {
                idx = *(unsigned char *)(b + 6);
                if (n - idx <= 0)
                    break;
                slot = sp + (idx << 6);
                f5[0] = SC0(ent, 0); f5[1] = SC0(ent, 1); f5[2] = SC0(ent, 2);
                ent = *(unsigned long *)(b + 8);
                b += 8;
                slotv = *(int *)(slot + 0x2C);
                ST4(slot + 0x20, f5[0], f5[1], f5[2], 1.0f);
                *(int *)(slot + 0x2C) = slotv;
                cB--;
            } while (cB > 0);
        }
        /* rotations straight from table 1, four joints per pass */
        k = n;
        w = sp - 0x100;
        a = sp + 0x2000;
        do {
            short *h = (short *)a;
            short *g = (short *)(a + 0x10);
            w += 0x100;
            for (i = 0; i < 4; i++) {
                f5[i] = HF(h, i);
                f6[i] = HF(h, 4 + i);
                f7[i] = HF(g, i);
                f8[i] = HF(g, 4 + i);
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
        int rw3, rec1c, tbits;
        float v6[4], v7[4], v8[4], v9[4], v10[4], v11[4], v12[4], v14[4];

        spr_dma_254218(0x2000, *(int *)(r + 0x10), w3 * 3);
        end = sp + 0x2000 + ((w3 * 3) << 4);
        tbits = *(int *)(r + 8);
        x4 = *(float *)&tbits;
        recs = sp + 0x2000;
        nrec = *(char **)(r + 0x1C);
        *(int *)(end + 0x2C) = (int)(sp + 0x1E00);
        w4 = 1.0f - x4;
        dst = *(char **)(recs + 0x2C);
        LD4(v9, recs);
        LD4(v10, recs + 0x10);
        LD4(v11, recs + 0x20);
        LD4(v6, dst);
        LD4(v7, dst + 0x10);
        LD4(v8, dst + 0x20);
        do {
            if ((int)(sp + (n << 6)) - (int)dst <= 0)
                break;
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
            Q = rsq_254218(LSQ4(v12));
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
        float ax;

        r = rr;
        tg = *(char **)(r + 4);
        sel = *(unsigned char *)(r + 3);
        rr = *(char **)(r + 8);
        if ((int)(sp + (n << 6)) - (int)tg <= 0)
            continue;
        LD4(v6, tg);
        LD4(v8, tg + 0x10);
        LD4(v10, tg + 0x20);
        LD4(v7, r + 0x10);
        LD4(v9, r + 0x20);
        LD4(v11, r + 0x30);
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
            Q = rsq_254218(LSQ4(v14));
            for (i = 0; i < 4; i++)
                v6[i] = v14[i] * Q;
        }
        PUT4(tg, v6);
        PUT4(tg + 0x10, v8);
        PUT4(tg + 0x20, v10);
    }

    /* final pass: quaternion to matrix, scale, concatenate with the parent matrix (slot+0..0x30) */
    {
        float P4[4] = {1.0f, 0.0f, 0.0f, 0.0f};
        float P5[4] = {0.0f, 1.0f, 0.0f, 0.0f};
        float P6[4] = {0.0f, 0.0f, 1.0f, 0.0f};
        float P7[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        float q[4], t[4], sc[4], tr[4];
        float a12[3], a13[3], a14y, a14z, a15z;
        float c16[4], c17[4], c18[4], c19[4];
        char *cur = sp;
        char *endp = sp + (n << 6);
        char *nxtp;

        *(int *)(endp + 0x2C) = (int)sp;
        do {
            LD4(q, cur);
            for (i = 0; i < 4; i++)
                t[i] = q[i] + q[i];
            c16[0] = 1.0f; c16[1] = 0.0f; c16[2] = 0.0f; c16[3] = 0.0f;
            c17[0] = 0.0f; c17[1] = 1.0f; c17[2] = 0.0f; c17[3] = 0.0f;
            c18[0] = 0.0f; c18[1] = 0.0f; c18[2] = 1.0f; c18[3] = 0.0f;
            c19[0] = 0.0f; c19[1] = 0.0f; c19[2] = 0.0f; c19[3] = 1.0f;
            LD4(sc, cur + 0x10);
            for (i = 0; i < 3; i++) {
                a12[i] = t[i] * q[3];
                a13[i] = t[i] * q[0];
            }
            a14y = t[1] * q[1];
            a14z = t[2] * q[1];
            a15z = t[2] * q[2];
            c17[0] = 0.0f + a12[2];
            c18[0] = 0.0f - a12[1];
            c18[1] = 0.0f + a12[0];
            c16[0] = c16[0] - a14y;
            c17[1] = c17[1] - a13[0];
            c18[2] = c18[2] - a13[0];
            c16[1] = a13[1] - a12[2];
            c16[2] = a13[2] + a12[1];
            c17[2] = a14z - a12[0];
            c17[0] = c17[0] + a13[1];
            c18[0] = c18[0] + a13[2];
            c18[1] = c18[1] + a14z;
            c16[0] = c16[0] - a15z;
            c17[1] = c17[1] - a15z;
            c18[2] = c18[2] - a14y;
            LD4(tr, cur + 0x20);
            for (i = 0; i < 3; i++) {
                c16[i] = c16[i] * sc[0];
                c17[i] = c17[i] * sc[1];
                c18[i] = c18[i] * sc[2];
                c19[i] = c19[i] + tr[i];
            }
            nxtp = cur + 0x40;
            {
                float o16[4], o17[4], o18[4], o19[4];
                for (i = 0; i < 4; i++) {
                    o16[i] = P4[i] * c16[0] + P5[i] * c16[1] + P6[i] * c16[2];
                    o17[i] = P4[i] * c17[0] + P5[i] * c17[1] + P6[i] * c17[2];
                    o18[i] = P4[i] * c18[0] + P5[i] * c18[1] + P6[i] * c18[2];
                    o19[i] = ((P4[i] * c19[0] + P5[i] * c19[1]) + P6[i] * c19[2]) + P7[i] * c19[3];
                }
                PUT4(cur, o16);
                PUT4(cur + 0x10, o17);
                PUT4(cur + 0x20, o18);
                /* the next joint's parent matrix: its first column is read before this slot's last column is
                   stored, the other three after (the parent is often this very slot) */
                {
                    char *par = *(char **)(nxtp + 0x2C);
                    LD4(P4, par);
                    PUT4(cur + 0x30, o19);
                    LD4(P5, par + 0x10);
                    LD4(P6, par + 0x20);
                    LD4(P7, par + 0x30);
                }
            }
            cur = nxtp;
        } while (cur != endp);
    }

    /* pack the joint translations (slot+0x30) to the start of the scratchpad, and to a2 */
    {
        int outp = out_;
        char *src = sp;
        char *dstp = sp - 0x20;
        float A[4], B[4];

        k = n;
        if (outp > 0) {
            do {
                LD4(A, src + 0x30);
                dstp += 0x20;
                LD4(B, src + 0x70);
                src += 0x80;
                PUT4(dstp, A);
                outp += 0x20;
                PUT4(dstp + 0x10, B);
                k -= 2;
                PUT4((char *)outp - 0x20, A);
                if (k < 0)
                    break;
                PUT4((char *)outp - 0x10, B);
            } while (k > 0);
        } else if (outp == 0) {
            do {
                LD4(A, src + 0x30);
                dstp += 0x20;
                LD4(B, src + 0x70);
                src += 0x80;
                PUT4(dstp, A);
                k -= 2;
                PUT4(dstp + 0x10, B);
            } while (k > 0);
        }
    }
}
