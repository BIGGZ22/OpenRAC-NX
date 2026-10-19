/* functional: medium. fcheck: unchecked (not listed for the port, so OpenRAC tools/fcheck_gate.py gave no verdict). MobyAnimEval(job, scratch): turns a moby's two animation keys into its joint palette in the scratchpad (record j at 0x70000000 + 0x40*j, class skeleton at 0x70001C00). Every DMA step (common translations, the two key payloads, the pose-layer entries, the skeleton) is a copy made where retail starts it, leaving the channel idle (no DMA controller in the port, and the completion waits on -1 sentinels would spin), so the sentinel stores and the DMA tags at scratch+0x1FC0 are dropped. The VU0 macro arithmetic is written lane by lane in retail's operation order (separate multiply and add roundings, ACC chains kept; vrsqrt takes |x| and gives the largest float for 0 instead of infinity), including retail's quirks: the post-scale list re-appends B records, the pose-layer and modifier blends always run their delay-slot vsub (so the difference is used as the sum when the dot is non-negative), pairs of joints are processed two at a time (odd counts touch record jc), and common translations are copied four joints at a time. Not run against retail. */
extern float sqrtf(float);

static float bits2f(unsigned int u) {
    union { unsigned int u; float f; } c;
    c.u = u;
    return c.f;
}

static unsigned int f2bits(float f) {
    union { unsigned int u; float f; } c;
    c.f = f;
    return c.u;
}

#define W(p, o) (*(unsigned int *)((char *)(p) + (o)))
#define FL(p, o) (*(float *)((char *)(p) + (o)))
#define Q15(v) ((float)(int)(v) * (1.0f / 32768.0f))

static float vrsq(float x) {
    if (x < 0.0f) {
        x = -x;
    }
    if (x == 0.0f) {
        return 3.4028235e38f;
    }
    return 1.0f / sqrtf(x);
}

static void put4(char *p, float x, float y, float z, float w) {
    float *q = (float *)p;
    q[0] = x;
    q[1] = y;
    q[2] = z;
    q[3] = w;
}

static void put3(char *p, float x, float y, float z) {
    float *q = (float *)p;
    q[0] = x;
    q[1] = y;
    q[2] = z;
}

static void copy16(char *dst, char *src) {
    unsigned int *d = (unsigned int *)dst;
    unsigned int *s = (unsigned int *)src;
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
}

/* qwc quadwords from main memory to the scratchpad, at once */
static void spr_copy(char *dst, unsigned int src, int qwc) {
    unsigned int *d = (unsigned int *)dst;
    unsigned int *s = (unsigned int *)src;
    int i;

    for (i = 0; i < qwc * 4; i++) {
        d[i] = s[i];
    }
    *(volatile unsigned int *)0x1000D400 = 0;
}

/* sum of squares in retail's order: ((x*x + y*y) + z*z) + w*w */
static float lsq4(float *v) {
    return (((v[0] * v[0]) + (v[1] * v[1])) + (v[2] * v[2])) + (v[3] * v[3]);
}

void func_0020EF30(char *job, char *scr) {
    char *spr = (char *)0x70000000;
    int jc = *(short *)(job + 0x10);
    unsigned int tbits;
    int flags;
    float t, u;
    char *common, *Aq, *Bq, *As, *At, *Bs, *Bt, *post;
    char *tailS, *tailP, *tailT;
    char *layers, *mods;
    char *s, *rec, *p;
    int j, k, n;

    if (jc == 0) {
        put4(spr, 1.0f, 0.0f, 0.0f, 0.0f);
        put4(spr + 0x10, 0.0f, 1.0f, 0.0f, 0.0f);
        put4(spr + 0x20, 0.0f, 0.0f, 1.0f, 0.0f);
        put4(spr + 0x30, 0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    tbits = W(job, 4);
    flags = *(short *)(job + 0x12);
    layers = (char *)W(job, 8);
    mods = (char *)W(job, 0xC);
    t = bits2f(tbits);
    u = 1.0f - t;

    common = spr + 0x2000;
    Aq = common + jc * 16;
    Bq = Aq + (int)W(job, 0x24) * 16;
    As = Aq + *(unsigned short *)(job + 0x28);
    At = Aq + *(unsigned short *)(job + 0x2C);
    Bs = Bq + *(unsigned short *)(job + 0x38);
    Bt = Bq + *(unsigned short *)(job + 0x3C);
    post = scr + 0x10;

    /* DMA chain: common translations, key A, and key B only when t != 0 */
    spr_copy(common, W(job, 0x18), jc);
    spr_copy(Aq, W(job, 0x20), (int)W(job, 0x24));
    if (tbits != 0) {
        spr_copy(Bq, W(job, 0x30), (int)W(job, 0x34));
    }

    W(scr, 0) = 0;
    W(scr, 4) = 0;
    W(scr, 8) = 0;
    W(scr, 0xC) = 0;

    tailS = scr - 0xC;   /* link word at +0xC: scr+0 (inherited-scale list) */
    tailP = scr;         /* link word at +0xC: scr+0xC (post-scale list) */
    tailT = scr - 0x38;  /* link word at +0x3C: scr+4 (translation list) */

    if (tbits != 0) {
        /* ---- two keys ---- */
        for (j = 0; j < jc; j++) {
            rec = spr + j * 0x40;
            put4(rec, 1.0f, 1.0f, 1.0f, 0.0f);
            put4(rec + 0x10, 1.0f, 1.0f, 1.0f, 0.0f);
            p = post + j * 0x20;
            put4(p, 1.0f, 1.0f, 1.0f, 0.0f);
            put4(p + 0x10, 1.0f, 1.0f, 1.0f, 0.0f);
        }
        for (k = 0; k < jc; k += 4) {
            for (j = k; j < k + 4; j++) {
                rec = spr + j * 0x40;
                copy16(rec + 0x20, common + j * 16);
                copy16(rec + 0x30, common + j * 16);
                W(rec, 0x3C) = 0;
            }
        }

        /* key A scale records */
        for (s = As; s != At; s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            unsigned int h3 = hi >> 16;
            int joint = h3 & 0xFF;
            float sx = Q15((lo & 0xFFFF) << 3);
            float sy = Q15((lo >> 16) << 3);
            float sz = Q15((hi & 0xFFFF) << 3);
            if (h3 & 0x8000) {
                rec = spr + joint * 0x40;
                W(tailS, 0xC) = (unsigned int)rec;
                tailS = rec;
                put3(rec, sx, sy, sz);
                W(rec, 0xC) = h3 << 3;
            } else {
                p = post + joint * 0x20;
                W(tailP, 0xC) = (unsigned int)p;
                tailP = p;
                put3(p, sx, sy, sz);
                W(p, 0xC) = h3 << 3;
            }
        }

        /* key A translation records */
        n = *(short *)(job + 0x2E);
        for (s = At; n != 0; n--, s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            int joint = (signed char)(hi >> 16);
            unsigned int par;
            if (joint < 0) {
                continue;
            }
            rec = spr + joint * 0x40;
            par = W(rec, 0x2C);
            W(tailT, 0x3C) = (unsigned int)rec;
            tailT = rec;
            W(rec, 0x3C) = 0;
            put3(rec + 0x20, (float)(short)lo, (float)(short)(lo >> 16), (float)(short)hi);
            W(rec, 0x2C) = (unsigned int)(int)(short)(hi >> 16);
            W(rec, 0x2C) = par;
        }

        /* key B scale records */
        for (s = Bs; s != Bt; s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            unsigned int h3 = hi >> 16;
            int joint = h3 & 0xFF;
            float sx = Q15((lo & 0xFFFF) << 3);
            float sy = Q15((lo >> 16) << 3);
            float sz = Q15((hi & 0xFFFF) << 3);
            unsigned int old;
            if (h3 & 0x8000) {
                rec = spr + joint * 0x40;
                old = W(rec, 0xC);
                put3(rec + 0x10, sx, sy, sz);
                W(rec, 0x1C) = h3 << 3;
                if (old == 0) {
                    W(tailS, 0xC) = (unsigned int)rec;
                    tailS = rec;
                }
            } else {
                p = post + joint * 0x20;
                old = W(p, 0x1C);
                put3(p + 0x10, sx, sy, sz);
                W(p, 0x1C) = h3 << 3;
                if (old == 0) {
                    W(tailP, 0xC) = (unsigned int)p;
                    tailP = p;
                }
            }
        }
        W(tailS, 0xC) = 0;
        W(tailP, 0xC) = 0;

        /* key B translation records */
        n = *(short *)(job + 0x3E);
        for (s = Bt; n != 0; n--, s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            int joint = (signed char)(hi >> 16);
            unsigned int old;
            if (joint < 0) {
                continue;
            }
            rec = spr + joint * 0x40;
            old = W(rec, 0x3C);
            put3(rec + 0x30, (float)(short)lo, (float)(short)(lo >> 16), (float)(short)hi);
            W(rec, 0x3C) = old;
            if (old == 0) {
                W(tailT, 0x3C) = (unsigned int)rec;
                tailT = rec;
                W(rec, 0x3C) = 0;
            }
        }

        /* blend the inherited scales and the translations of the listed joints */
        for (rec = (char *)W(scr, 0); rec != 0;) {
            float *a = (float *)rec;
            float *b = (float *)(rec + 0x10);
            unsigned int next = W(rec, 0xC);
            put4(rec + 0x10, a[0] * u + b[0] * t, a[1] * u + b[1] * t, a[2] * u + b[2] * t, 1.0f);
            rec = (char *)next;
        }
        for (rec = (char *)W(scr, 4); rec != 0;) {
            float *a = (float *)(rec + 0x20);
            float *b = (float *)(rec + 0x30);
            unsigned int par = W(rec, 0x2C);
            put4(rec + 0x20, a[0] * u + b[0] * t, a[1] * u + b[1] * t, a[2] * u + b[2] * t, 1.0f);
            W(rec, 0x2C) = par;
            rec = (char *)W(rec, 0x3C);
        }

        /* rotations, two joints at a time */
        {
            char *qa = Aq, *qb = Bq, *out = spr;
            for (n = jc; n > 0; n -= 2) {
                int m;
                for (m = 0; m < 2; m++) {
                    float a[4], b[4], pos[4], neg[4], q[4], d, Q;
                    int i;
                    for (i = 0; i < 4; i++) {
                        a[i] = Q15(((short *)qa)[m * 4 + i]);
                        b[i] = Q15(((short *)qb)[m * 4 + i]);
                    }
                    if (flags & 0x400) {
                        for (i = 0; i < 4; i++) {
                            q[i] = a[i] * u + b[i] * t;
                        }
                    } else {
                        for (i = 0; i < 4; i++) {
                            pos[i] = a[i] * u + b[i] * t;
                            neg[i] = a[i] * u + (-b[i]) * t;
                        }
                        d = (((a[1] * b[1]) + (a[0] * b[0])) + (a[2] * b[2])) + (a[3] * b[3]);
                        if ((f2bits(d) >> 31) == 0) {
                            for (i = 0; i < 4; i++) {
                                q[i] = pos[i];
                            }
                        } else {
                            for (i = 0; i < 4; i++) {
                                q[i] = neg[i];
                            }
                        }
                        Q = vrsq(lsq4(q));
                        for (i = 0; i < 4; i++) {
                            q[i] = q[i] * Q;
                        }
                    }
                    put4(out + m * 0x40, q[0], q[1], q[2], q[3]);
                }
                qa += 16;
                qb += 16;
                out += 0x80;
            }
        }
    } else {
        /* ---- one key (t == 0) ---- */
        for (j = 0; j < jc; j++) {
            put4(spr + j * 0x40 + 0x10, 1.0f, 1.0f, 1.0f, 0.0f);
            put4(post + j * 0x20, 1.0f, 1.0f, 1.0f, 0.0f);
        }
        for (k = 0; k < jc; k += 4) {
            for (j = k; j < k + 4; j++) {
                copy16(spr + j * 0x40 + 0x20, common + j * 16);
            }
        }

        for (s = As; s != At; s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            unsigned int h3 = hi >> 16;
            int joint = h3 & 0xFF;
            float sx = Q15((lo & 0xFFFF) << 3);
            float sy = Q15((lo >> 16) << 3);
            float sz = Q15((hi & 0xFFFF) << 3);
            if (h3 & 0x8000) {
                rec = spr + joint * 0x40;
                put3(rec + 0x10, sx, sy, sz);
                W(rec, 0x1C) = h3 << 3;
            } else {
                p = post + joint * 0x20;
                W(tailP, 0xC) = (unsigned int)p;
                tailP = p;
                put3(p, sx, sy, sz);
                W(p, 0xC) = h3 << 3;
            }
        }
        W(tailP, 0xC) = 0;

        n = *(short *)(job + 0x2E);
        for (s = At; n != 0; n--, s += 8) {
            unsigned int lo = W(s, 0), hi = W(s, 4);
            int joint = (signed char)(hi >> 16);
            unsigned int par;
            if (joint < 0) {
                continue;
            }
            rec = spr + joint * 0x40;
            par = W(rec, 0x2C);
            put3(rec + 0x20, (float)(short)lo, (float)(short)(lo >> 16), (float)(short)hi);
            W(rec, 0x2C) = par;
        }

        for (j = 0; j < jc; j++) {
            short *q = (short *)(Aq + j * 8);
            put4(spr + j * 0x40, Q15(q[0]), Q15(q[1]), Q15(q[2]), Q15(q[3]));
        }
    }

    /* ---- pose layers (job+8): blend whole joint records in by a weight ---- */
    while (layers != 0) {
        int cnt = *(unsigned short *)(layers + 2);
        float wt = FL(layers, 8);
        float omw = 1.0f - wt;
        char *e = spr + 0x2000;
        char *end = e + 48 * cnt;

        spr_copy(e, W(layers, 0x10), 3 * cnt);
        layers = (char *)W(layers, 0x1C);
        do {
            char *tgt = (char *)W(e, 0x2C);
            float *eq = (float *)e, *es = (float *)(e + 0x10), *et = (float *)(e + 0x20);
            float *cq = (float *)tgt, *cs = (float *)(tgt + 0x10), *ct = (float *)(tgt + 0x20);
            float a[4], b[4], sum[4], diff[4], prod[4], d, ls, Q;
            float sc0, sc1, sc2, t0, t1, t2;
            unsigned int escalew = W(e, 0x1C);
            int i;

            for (i = 0; i < 4; i++) {
                a[i] = cq[i] * omw;
                b[i] = eq[i] * wt;
                sum[i] = a[i] + b[i];
                prod[i] = a[i] * b[i];
                diff[i] = a[i] - b[i];
            }
            sc0 = cs[0] * omw + es[0] * wt;
            sc1 = cs[1] * omw + es[1] * wt;
            sc2 = cs[2] * omw + es[2] * wt;
            t0 = ct[0] * omw + et[0] * wt;
            t1 = ct[1] * omw + et[1] * wt;
            t2 = ct[2] * omw + et[2] * wt;
            d = (((prod[1] + prod[0]) + prod[2]) + prod[3]);
            ls = lsq4(sum);
            if ((f2bits(d) >> 31) != 0) {
                ls = lsq4(diff);
            }
            Q = vrsq(ls);
            e += 0x30;
            put3(tgt + 0x10, sc0, sc1, sc2);
            put3(tgt + 0x20, t0, t1, t2);
            W(tgt, 0x1C) = escalew;
            put4(tgt, diff[0] * Q, diff[1] * Q, diff[2] * Q, diff[3] * Q);
        } while (e != end);
    }

    /* ---- joint modifiers (job+0xC) ---- */
    while (mods != 0) {
        char *node = mods;
        char *tgt = (char *)W(node, 4);
        int mode = *(unsigned char *)(node + 3);
        float *a = (float *)tgt;
        float *b = (float *)(node + 0x10);
        float *ts = (float *)(tgt + 0x10), *ns = (float *)(node + 0x20);
        float *tt = (float *)(tgt + 0x20), *nt = (float *)(node + 0x30);
        float r[4], s3[3], tr[3];
        int i;

        mods = (char *)W(node, 8);
        if (mode == 0) {
            float c[3], dt, rw;
            c[0] = a[1] * b[2] - b[1] * a[2];
            c[1] = a[2] * b[0] - b[2] * a[0];
            c[2] = a[0] * b[1] - b[0] * a[1];
            dt = ((b[0] * a[0]) + (b[1] * a[1])) + (b[2] * a[2]);
            rw = (b[3] * a[3]) - dt;
            for (i = 0; i < 3; i++) {
                r[i] = ((b[i] * a[3]) + (a[i] * b[3])) + c[i];
                s3[i] = ts[i] * ns[i];
                tr[i] = tt[i] + nt[i];
            }
            r[3] = rw;
        } else {
            float wt = FL(node, 0xC);
            float omw = 1.0f - wt;
            float aa[4], bb[4], sum[4], diff[4], prod[4], d, ls, Q;
            for (i = 0; i < 4; i++) {
                aa[i] = a[i] * omw;
                bb[i] = b[i] * wt;
                sum[i] = aa[i] + bb[i];
                prod[i] = aa[i] * bb[i];
                diff[i] = aa[i] - bb[i];
            }
            for (i = 0; i < 3; i++) {
                s3[i] = ts[i] * omw + ns[i] * wt;
                tr[i] = tt[i] * omw + nt[i] * wt;
            }
            d = (((prod[1] + prod[0]) + prod[2]) + prod[3]);
            ls = lsq4(sum);
            if ((f2bits(d) >> 31) != 0) {
                ls = lsq4(diff);
            }
            Q = vrsq(ls);
            for (i = 0; i < 4; i++) {
                r[i] = diff[i] * Q;
            }
        }
        {
            unsigned int tw = W(tgt, 0x2C);
            put4(tgt, r[0], r[1], r[2], r[3]);
            put4(tgt + 0x10, s3[0], s3[1], s3[2], 1.0f);
            put3(tgt + 0x20, tr[0], tr[1], tr[2]);
            W(tgt, 0x2C) = tw;
        }
    }

    /* class skeleton to 0x70001C00 */
    spr_copy(spr + 0x1C00, W(job, 0x1C), jc * 4);

    /* local matrix of each joint, parents first */
    for (j = 0; j < jc; j++) {
        char *rj = spr + j * 0x40;
        float *q = (float *)rj;
        float x = q[0], y = q[1], z = q[2], w = q[3];
        float x2 = x + x, y2 = y + y, z2 = z + z, w2 = w + w;
        float a0 = x2 * w, a1 = y2 * w, a2 = z2 * w;
        float b0 = x2 * x, b1 = y2 * x, b2 = z2 * x;
        float c1 = y2 * y, c2 = z2 * y;
        float d2 = z2 * z;
        float r0[4], r1[4], r2[4], P[4][4], ro[4][4], tx, ty, tz;
        unsigned int sflag = W(rj, 0x1C);
        unsigned int par = W(rj, 0x2C);
        float *sc = (float *)(rj + 0x10);
        float *tv = (float *)(rj + 0x20);
        int l;

        (void)w2;
        r0[0] = (1.0f - c1) - d2;
        r0[1] = b1 - a2;
        r0[2] = b2 + a1;
        r0[3] = 0.0f;
        r1[0] = (0.0f + a2) + b1;
        r1[1] = (1.0f - b0) - d2;
        r1[2] = c2 - a0;
        r1[3] = 0.0f;
        r2[0] = (0.0f - a1) + b2;
        r2[1] = (0.0f + a0) + c2;
        r2[2] = (1.0f - b0) - c1;
        r2[3] = 0.0f;
        if (sflag != 0) {
            for (l = 0; l < 3; l++) {
                r0[l] = r0[l] * sc[0];
                r1[l] = r1[l] * sc[1];
                r2[l] = r2[l] * sc[2];
            }
        }
        tx = 0.0f + tv[0];
        ty = 0.0f + tv[1];
        tz = 0.0f + tv[2];
        if (par != 0) {
            float *pp = (float *)par;
            for (l = 0; l < 16; l++) {
                P[l >> 2][l & 3] = pp[l];
            }
        } else {
            for (l = 0; l < 16; l++) {
                P[l >> 2][l & 3] = ((l >> 2) == (l & 3)) ? 1.0f : 0.0f;
            }
        }
        for (l = 0; l < 4; l++) {
            ro[0][l] = (P[0][l] * r0[0] + P[1][l] * r0[1]) + P[2][l] * r0[2];
            ro[1][l] = (P[0][l] * r1[0] + P[1][l] * r1[1]) + P[2][l] * r1[2];
            ro[2][l] = (P[0][l] * r2[0] + P[1][l] * r2[1]) + P[2][l] * r2[2];
            ro[3][l] = ((P[0][l] * tx + P[1][l] * ty) + P[2][l] * tz) + P[3][l] * 1.0f;
        }
        put4(rj, ro[0][0], ro[0][1], ro[0][2], ro[0][3]);
        put4(rj + 0x10, ro[1][0], ro[1][1], ro[1][2], ro[1][3]);
        put4(rj + 0x20, ro[2][0], ro[2][1], ro[2][2], ro[2][3]);
        put4(rj + 0x30, ro[3][0], ro[3][1], ro[3][2], ro[3][3]);
    }

    /* post-scale: scale the rows of the listed joints after the whole chain */
    for (p = (char *)W(scr, 0xC); p != 0;) {
        float *sa = (float *)p;
        float *sb = (float *)(p + 0x10);
        float s0, s1, s2;
        unsigned int next = W(p, 0xC);
        char *rj = spr + ((p - post) << 1);
        float *m;

        if (tbits != 0) {
            s0 = sa[0] * u + sb[0] * t;
            s1 = sa[1] * u + sb[1] * t;
            s2 = sa[2] * u + sb[2] * t;
        } else {
            s0 = sa[0];
            s1 = sa[1];
            s2 = sa[2];
        }
        m = (float *)rj;
        m[0] = m[0] * s0;
        m[1] = m[1] * s0;
        m[2] = m[2] * s0;
        m = (float *)(rj + 0x10);
        m[0] = m[0] * s1;
        m[1] = m[1] * s1;
        m[2] = m[2] * s1;
        m = (float *)(rj + 0x20);
        m[0] = m[0] * s2;
        m[1] = m[1] * s2;
        m[2] = m[2] * s2;
        p = (char *)next;
    }

    /* palette: F = P * skeleton (the w column of the skeleton is never read) */
    for (j = 0; j < jc; j++) {
        char *rj = spr + j * 0x40;
        float *S = (float *)(spr + 0x1C00 + j * 0x40);
        float *P0 = (float *)rj, *P1 = (float *)(rj + 0x10), *P2 = (float *)(rj + 0x20), *P3 = (float *)(rj + 0x30);
        float F[4][4];
        int l, i;

        for (l = 0; l < 4; l++) {
            for (i = 0; i < 3; i++) {
                F[i][l] = (P0[l] * S[i * 4 + 0] + P1[l] * S[i * 4 + 1]) + P2[l] * S[i * 4 + 2];
            }
            F[3][l] = ((P0[l] * S[12] + P1[l] * S[13]) + P2[l] * S[14]) + P3[l] * 1.0f;
        }
        for (i = 0; i < 4; i++) {
            put4(rj + i * 0x10, F[i][0], F[i][1], F[i][2], F[i][3]);
        }
    }
}
