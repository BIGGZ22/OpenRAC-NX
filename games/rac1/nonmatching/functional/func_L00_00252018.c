/* functional: medium. fcheck: unchecked (not listed for the port, so OpenRAC tools/fcheck_gate.py gave no verdict). MobyAnimEval(job, scratch): evaluates one animated moby's skeleton in the scratchpad (0x70000000, joint j at 0x40*j). Retail chains three DMA transfers to the scratchpad at 0x2000 (common translations, key A, key B), then the 0x60 layer list entries, then the skeleton at 0x1C00; each is a copy made where retail starts it (the port has no DMA controller; the polls on the -1 sentinels are dropped) and the channel is left idle. The rest follows retail step by step with the VU0 lanes written out: records of key A/B into the joint slots and the linked lists in scratch (inherited scale, translation, post-scale), lerp of the lists by t, quaternion lerp (normalised, sign-flipped by the dot test, unless job flag 0x400 or t == 0), the 0x60 / 0x64 runtime layers, the local matrices chained through the parent words, the post-scale rows and the palette (joint matrix times the skeleton rows). Not run against retail, hand-checked against the assembly only; vrsqrt is 1 / sqrtf (FLT_MAX for 0). A zero entry count in a 0x60 node would make retail's do-while run off; here it is skipped. */
extern float sqrtf(float);
extern void func_L00_00252CEC(void);

#define F32(p) (*(float *)(p))
#define I32(p) (*(int *)(p))
#define ST4(p, a, b, c, d) do { float *_p = (float *)(p); _p[0] = (a); _p[1] = (b); _p[2] = (c); _p[3] = (d); } while (0)
#define LD4(v, p) do { float *_p = (float *)(p); (v)[0] = _p[0]; (v)[1] = _p[1]; (v)[2] = _p[2]; (v)[3] = _p[3]; } while (0)
#define PUT4(p, v) ST4(p, (v)[0], (v)[1], (v)[2], (v)[3])
#define PUT3(p, v) do { float *_p = (float *)(p); _p[0] = (v)[0]; _p[1] = (v)[1]; _p[2] = (v)[2]; } while (0)
#define UCF(e, i) ((float)((int)(((e) >> (16 * (i))) & 0xFFFF) << 3) / 32768.0f)
#define SCF(e, i) ((float)(short)((e) >> (16 * (i))))
#define QH(h) ((float)(short)(h) / 32768.0f)

static void spr_copy_252018(char *dst, char *src, int qwc)
{
    int *d = (int *)dst;
    int *s = (int *)src;
    int i;

    for (i = 0; i < qwc * 4; i++)
        d[i] = s[i];
    *(volatile unsigned int *)0x1000D400 = 0;
}

static float rsq_252018(float x)
{
    if (x <= 0.0f)
        return 3.4028235e38f;
    return 1.0f / sqrtf(x);
}

static int neg_252018(float x)
{
    return (int)((*(unsigned int *)&x) >> 31);
}

/* ((v0 + v1) + v2) + v3 */
static float sum4_252018(float *v)
{
    return ((v[0] + v[1]) + v[2]) + v[3];
}

/* r = a * u + b * t on all four lanes; with norm, the dot test picks b or -b and the result is normalised */
static void qlerp_252018(float *a, float *b, float u, float t, int norm, float *r)
{
    float prod[4], sq[4], v[4];
    float dd, Q;
    int i;

    if (!norm) {
        for (i = 0; i < 4; i++)
            r[i] = a[i] * u + b[i] * t;
        return;
    }
    for (i = 0; i < 4; i++)
        prod[i] = a[i] * b[i];
    dd = ((prod[1] + prod[0]) + prod[2]) + prod[3];
    if (!neg_252018(dd)) {
        for (i = 0; i < 4; i++)
            v[i] = a[i] * u + b[i] * t;
    } else {
        for (i = 0; i < 4; i++)
            v[i] = a[i] * u + (b[i] * -1.0f) * t;
    }
    for (i = 0; i < 4; i++)
        sq[i] = v[i] * v[i];
    Q = rsq_252018(sum4_252018(sq));
    for (i = 0; i < 4; i++)
        r[i] = v[i] * Q;
}

void func_L00_00252018(char *job, char *scratch)
{
    char *sp = (char *)0x70000000;
    int jc = *(short *)(job + 0x10);
    int flags;
    unsigned int tbits;
    float t, u;
    int two;
    char *A, *B, *Ar, *Br, *ct;
    int qa, qb, o28, o2c, o38, o3c;
    char *p11, *p16, *tail, *r, *slot, *post, *d, *tgt, *nxt;
    unsigned long e;
    int j, k, cnt, jt, pw, lk, wr;
    float v[4], f[4];

    if (jc == 0) {
        func_L00_00252CEC();
        return;
    }
    tbits = *(unsigned int *)(job + 4);
    t = *(float *)(job + 4);
    u = 1.0f - t;
    two = (tbits != 0);
    flags = *(short *)(job + 0x12);
    qa = *(int *)(job + 0x24);
    qb = *(int *)(job + 0x34);
    o28 = *(unsigned short *)(job + 0x28);
    o2c = *(unsigned short *)(job + 0x2C);
    o38 = *(unsigned short *)(job + 0x38);
    o3c = *(unsigned short *)(job + 0x3C);

    /* DMA chain: common translations (jc), key A (qa), key B (qb, only when t != 0) to 0x2000 */
    ct = sp + 0x2000;
    A = ct + jc * 16;
    B = A + ((unsigned int)qa << 4);
    spr_copy_252018(ct, *(char **)(job + 0x18), jc);
    spr_copy_252018(A, *(char **)(job + 0x20), qa & 0xFFFF);
    if (two)
        spr_copy_252018(B, *(char **)(job + 0x30), qb & 0xFFFF);

    /* list heads */
    I32(scratch + 0) = 0;
    I32(scratch + 4) = 0;
    I32(scratch + 8) = 0;
    I32(scratch + 12) = 0;

    /* defaults: slot +0 / +0x10 = (1,1,1,0), post-scale records A / B = (1,1,1,0) */
    j = jc;
    k = 0;
    do {
        if (two) {
            ST4(sp + k * 0x40, 1.0f, 1.0f, 1.0f, 0.0f);
            ST4(sp + k * 0x40 + 0x10, 1.0f, 1.0f, 1.0f, 0.0f);
            ST4(scratch + 0x10 + k * 0x20, 1.0f, 1.0f, 1.0f, 0.0f);
            ST4(scratch + 0x20 + k * 0x20, 1.0f, 1.0f, 1.0f, 0.0f);
        } else {
            ST4(sp + k * 0x40 + 0x10, 1.0f, 1.0f, 1.0f, 0.0f);
            ST4(scratch + 0x10 + k * 0x20, 1.0f, 1.0f, 1.0f, 0.0f);
        }
        k++;
        j--;
    } while (j > 0);

    /* common translations into slot +0x20 (+0x30 too and its link word cleared when blending), four joints per pass */
    j = jc;
    d = sp - 0x100;
    r = ct;
    do {
        d += 0x100;
        for (k = 0; k < 4; k++) {
            LD4(v, r + 0x10 * k);
            PUT4(d + 0x20 + 0x40 * k, v);
        }
        if (two) {
            for (k = 0; k < 4; k++) {
                LD4(v, r + 0x10 * k);
                PUT4(d + 0x30 + 0x40 * k, v);
            }
            for (k = 0; k < 4; k++)
                I32(d + 0x3C + 0x40 * k) = 0;
        }
        r += 0x40;
        j -= 4;
    } while (j > 0);

    p16 = scratch;
    if (two) {
        p11 = scratch - 0xC;

        /* key A scale records: bit 63 = inherited (slot +0), else post-scale (scratch record) */
        for (r = A + o28; r != A + o2c; r += 8) {
            e = *(unsigned long *)r;
            jt = *(unsigned char *)(r + 6);
            slot = sp + (jt << 6);
            post = scratch + 0x10 + (jt << 5);
            f[0] = UCF(e, 0); f[1] = UCF(e, 1); f[2] = UCF(e, 2);
            wr = (int)(((e >> 48) & 0xFFFF) << 3);
            if ((long)e < 0) {
                I32(p11 + 0xC) = (int)slot;
                p11 = slot;
                PUT3(slot, f);
                I32(slot + 0xC) = wr;
            } else {
                I32(p16 + 0xC) = (int)post;
                p16 = post;
                PUT3(post, f);
                I32(post + 0xC) = wr;
            }
        }
        /* key A translations into slot +0x20 (parent word kept), linked through +0x3C from scratch +4 */
        cnt = *(short *)(job + 0x2E);
        tail = scratch - 0x38;
        r = A + o2c;
        while (cnt != 0) {
            e = *(unsigned long *)r;
            jt = *(signed char *)(r + 6);
            cnt--;
            r += 8;
            slot = sp + (jt << 6);
            f[0] = SCF(e, 0); f[1] = SCF(e, 1); f[2] = SCF(e, 2);
            I32(tail + 0x3C) = (int)slot;
            tail = slot;
            I32(slot + 0x3C) = 0;
            PUT3(slot + 0x20, f);
        }
        /* key B scale records into +0x10 of the slot or of the post-scale record; linked on first touch */
        for (r = B + o38; r != B + o3c; r += 8) {
            e = *(unsigned long *)r;
            jt = *(unsigned char *)(r + 6);
            slot = sp + (jt << 6);
            post = scratch + 0x10 + (jt << 5);
            f[0] = UCF(e, 0); f[1] = UCF(e, 1); f[2] = UCF(e, 2);
            wr = (int)(((e >> 48) & 0xFFFF) << 3);
            if ((long)e < 0) {
                lk = I32(slot + 0xC);
                PUT3(slot + 0x10, f);
                I32(slot + 0x1C) = wr;
                if (lk == 0) {
                    I32(p11 + 0xC) = (int)slot;
                    p11 = slot;
                }
            } else {
                lk = I32(post + 0x1C);
                PUT3(post + 0x10, f);
                I32(post + 0x1C) = wr;
                if (lk == 0) {
                    I32(p16 + 0xC) = (int)post;
                    p16 = post;
                }
            }
        }
        I32(p11 + 0xC) = 0;
        I32(p16 + 0xC) = 0;
        /* key B translations into slot +0x30 */
        cnt = *(short *)(job + 0x3E);
        r = B + o3c;
        while (cnt != 0) {
            e = *(unsigned long *)r;
            jt = *(signed char *)(r + 6);
            cnt--;
            r += 8;
            slot = sp + (jt << 6);
            f[0] = SCF(e, 0); f[1] = SCF(e, 1); f[2] = SCF(e, 2);
            lk = I32(slot + 0x3C);
            PUT3(slot + 0x30, f);
            if (lk == 0) {
                I32(tail + 0x3C) = (int)slot;
                tail = slot;
                I32(slot + 0x3C) = 0;
            }
        }

        /* inherited scales: s = A * u + B * t, stored to slot +0x10 with w = 1 */
        d = *(char **)(scratch + 0);
        while (d != 0) {
            float a3[4], b3[4];

            LD4(a3, d);
            LD4(b3, d + 0x10);
            nxt = *(char **)(d + 0xC);
            ST4(d + 0x10, a3[0] * u + b3[0] * t, a3[1] * u + b3[1] * t, a3[2] * u + b3[2] * t, 1.0f);
            d = nxt;
        }
        /* translations: t = A * u + B * t into slot +0x20, parent word kept */
        d = *(char **)(scratch + 4);
        while (d != 0) {
            float a3[4], b3[4];

            LD4(a3, d + 0x20);
            LD4(b3, d + 0x30);
            pw = I32(d + 0x2C);
            ST4(d + 0x20, a3[0] * u + b3[0] * t, a3[1] * u + b3[1] * t, a3[2] * u + b3[2] * t, 1.0f);
            I32(d + 0x2C) = pw;
            d = *(char **)(d + 0x3C);
        }

        /* rotations: two joints per pass, written to slot +0 */
        j = jc;
        r = A;
        tail = B;
        d = sp - 0x80;
        while (j > 0) {
            float qa0[4], qb0[4], qa1[4], qb1[4], o0[4], o1[4];
            short *ha = (short *)r;
            short *hb = (short *)tail;

            j -= 2;
            for (k = 0; k < 4; k++) {
                qa0[k] = QH(ha[k]);
                qa1[k] = QH(ha[4 + k]);
                qb0[k] = QH(hb[k]);
                qb1[k] = QH(hb[4 + k]);
            }
            d += 0x80;
            r += 0x10;
            tail += 0x10;
            qlerp_252018(qa0, qb0, u, t, (flags & 0x400) == 0, o0);
            qlerp_252018(qa1, qb1, u, t, (flags & 0x400) == 0, o1);
            PUT4(d, o0);
            PUT4(d + 0x40, o1);
        }
    } else {
        /* single key: scale records go straight to slot +0x10 (inherited) or the post-scale list */
        for (r = A + o28; r != A + o2c; r += 8) {
            e = *(unsigned long *)r;
            jt = *(unsigned char *)(r + 6);
            slot = sp + (jt << 6);
            post = scratch + 0x10 + (jt << 5);
            f[0] = UCF(e, 0); f[1] = UCF(e, 1); f[2] = UCF(e, 2);
            wr = (int)(((e >> 48) & 0xFFFF) << 3);
            if ((long)e < 0) {
                PUT3(slot + 0x10, f);
                I32(slot + 0x1C) = wr;
            } else {
                I32(p16 + 0xC) = (int)post;
                p16 = post;
                PUT3(post, f);
                I32(post + 0xC) = wr;
            }
        }
        I32(p16 + 0xC) = 0;
        cnt = *(short *)(job + 0x2E);
        r = A + o2c;
        while (cnt != 0) {
            e = *(unsigned long *)r;
            jt = *(signed char *)(r + 6);
            cnt--;
            r += 8;
            slot = sp + (jt << 6);
            f[0] = SCF(e, 0); f[1] = SCF(e, 1); f[2] = SCF(e, 2);
            PUT3(slot + 0x20, f);
        }
        /* rotations straight from key A */
        for (k = 0; k < jc; k++) {
            short *ha = (short *)(A + k * 8);

            ST4(sp + k * 0x40, QH(ha[0]), QH(ha[1]), QH(ha[2]), QH(ha[3]));
        }
    }

    /* runtime layers, list at job +8: DMA the 0x30-byte entries to 0x2000, blend into their target joints */
    nxt = *(char **)(job + 8);
    while (nxt != 0) {
        int n = *(unsigned short *)(nxt + 2);
        float wgt = *(float *)(nxt + 8);
        float x20 = 1.0f - wgt;
        char *ent;
        char *endp;

        spr_copy_252018(sp + 0x2000, *(char **)(nxt + 0x10), 3 * n);
        endp = sp + 0x2000 + (3 * n) * 16;
        nxt = *(char **)(nxt + 0x1C);
        for (ent = sp + 0x2000; ent != endp; ent += 0x30) {
            float q[4], s[4], tr[4], eq[4], es[4], et[4], sq[4], c16[4], c18[4];
            float Q, dd;
            int sw;
            int i;

            tgt = *(char **)(ent + 0x2C);
            LD4(eq, ent);
            LD4(es, ent + 0x10);
            LD4(et, ent + 0x20);
            LD4(q, tgt);
            LD4(s, tgt + 0x10);
            LD4(tr, tgt + 0x20);
            for (i = 0; i < 4; i++) {
                q[i] = q[i] * x20;
                eq[i] = eq[i] * wgt;
            }
            for (i = 0; i < 3; i++) {
                s[i] = s[i] * x20;
                es[i] = es[i] * wgt;
                tr[i] = tr[i] * x20;
                et[i] = et[i] * wgt;
            }
            for (i = 0; i < 4; i++) {
                c16[i] = q[i] + eq[i];
                c18[i] = q[i] * eq[i];
            }
            for (i = 0; i < 3; i++) {
                s[i] = s[i] + es[i];
                tr[i] = tr[i] + et[i];
            }
            dd = ((c18[1] + c18[0]) + c18[2]) + c18[3];
            if (neg_252018(dd)) {
                for (i = 0; i < 4; i++)
                    c16[i] = q[i] - eq[i];
            }
            for (i = 0; i < 4; i++)
                sq[i] = c16[i] * c16[i];
            Q = rsq_252018(sum4_252018(sq));
            PUT3(tgt + 0x10, s);
            PUT3(tgt + 0x20, tr);
            sw = I32(tgt + 0x1C);
            if (sw == 0)
                I32(tgt + 0x1C) = *(int *)(ent + 0x1C);
            ST4(tgt, c16[0] * Q, c16[1] * Q, c16[2] * Q, c16[3] * Q);
        }
    }

    /* runtime layers, list at job +0xC: node = mode @+3, target @+4, next @+8, weight @+0xC, quat @+0x10, scale @+0x20, trans @+0x30 */
    nxt = *(char **)(job + 0xC);
    while (nxt != 0) {
        float a[4], b[4], s[4], sm[4], tr[4], tm[4], r4[4], s4[4];
        float Q, dd;
        int mode = *(unsigned char *)(nxt + 3);
        int i;

        tgt = *(char **)(nxt + 4);
        LD4(a, tgt);
        LD4(s, tgt + 0x10);
        LD4(tr, tgt + 0x20);
        LD4(b, nxt + 0x10);
        LD4(sm, nxt + 0x20);
        LD4(tm, nxt + 0x30);
        d = nxt;
        nxt = *(char **)(nxt + 8);
        if (mode == 0) {
            float prod[4], va[4], vb[4], cr[3];
            float dot3;

            for (i = 0; i < 3; i++) {
                prod[i] = b[i] * a[i];
                va[i] = b[i] * a[3];
                vb[i] = a[i] * b[3];
            }
            cr[0] = a[1] * b[2] - b[1] * a[2];
            cr[1] = a[2] * b[0] - b[2] * a[0];
            cr[2] = a[0] * b[1] - b[0] * a[1];
            dot3 = (prod[0] + prod[1]) + prod[2];
            r4[0] = (va[0] + vb[0]) + cr[0];
            r4[1] = (va[1] + vb[1]) + cr[1];
            r4[2] = (va[2] + vb[2]) + cr[2];
            r4[3] = b[3] * a[3] - dot3;
            ST4(tgt, r4[0], r4[1], r4[2], r4[3]);
            ST4(tgt + 0x10, s[0] * sm[0], s[1] * sm[1], s[2] * sm[2], 1.0f);
            F32(tgt + 0x20) = tr[0] + tm[0];
            F32(tgt + 0x24) = tr[1] + tm[1];
            F32(tgt + 0x28) = tr[2] + tm[2];
        } else {
            float wgt = *(float *)(d + 0xC);
            float x20 = 1.0f - wgt;
            float s3[4], c18[4], sq[4];

            for (i = 0; i < 4; i++) {
                a[i] = a[i] * x20;
                b[i] = b[i] * wgt;
            }
            for (i = 0; i < 3; i++) {
                s[i] = s[i] * x20;
                sm[i] = sm[i] * wgt;
                tr[i] = tr[i] * x20;
                tm[i] = tm[i] * wgt;
            }
            for (i = 0; i < 4; i++) {
                s4[i] = a[i] + b[i];
                c18[i] = a[i] * b[i];
            }
            for (i = 0; i < 3; i++) {
                s3[i] = s[i] + sm[i];
                tr[i] = tr[i] + tm[i];
            }
            dd = ((c18[1] + c18[0]) + c18[2]) + c18[3];
            if (neg_252018(dd)) {
                for (i = 0; i < 4; i++)
                    s4[i] = a[i] - b[i];
            }
            for (i = 0; i < 4; i++)
                sq[i] = s4[i] * s4[i];
            Q = rsq_252018(sum4_252018(sq));
            ST4(tgt, s4[0] * Q, s4[1] * Q, s4[2] * Q, s4[3] * Q);
            ST4(tgt + 0x10, s3[0], s3[1], s3[2], 1.0f);
            F32(tgt + 0x20) = tr[0];
            F32(tgt + 0x24) = tr[1];
            F32(tgt + 0x28) = tr[2];
        }
    }

    /* skeleton rows to 0x1C00 (4 quadwords per joint) */
    spr_copy_252018(sp + 0x1C00, *(char **)(job + 0x1C), jc * 4);

    /* local matrices, parents first: rows of the (scaled) rotation, translation, product with the parent matrix */
    slot = sp;
    for (j = 0; j < jc; j++, slot += 0x40) {
        float q[4], q2[4], w12[3], x13[3], y14[2], z15;
        float r16[4], r17[4], r18[4], tt[4], sc[4];
        float P0[4], P1[4], P2[4], P3[4], o0[4], o1[4], o2[4], o3[4];
        int sw, i;

        LD4(q, slot);
        for (i = 0; i < 4; i++)
            q2[i] = q[i] + q[i];
        for (i = 0; i < 3; i++) {
            w12[i] = q2[i] * q[3];
            x13[i] = q2[i] * q[0];
        }
        y14[0] = q2[1] * q[1];
        y14[1] = q2[2] * q[1];
        z15 = q2[2] * q[2];
        r16[0] = 1.0f; r16[1] = 0.0f; r16[2] = 0.0f; r16[3] = 0.0f;
        r17[0] = 0.0f; r17[1] = 1.0f; r17[2] = 0.0f; r17[3] = 0.0f;
        r18[0] = 0.0f; r18[1] = 0.0f; r18[2] = 1.0f; r18[3] = 0.0f;
        r17[0] = 0.0f + w12[2];
        r18[0] = 0.0f - w12[1];
        r18[1] = 0.0f + w12[0];
        r16[0] = r16[0] - y14[0];
        r17[1] = r17[1] - x13[0];
        r18[2] = r18[2] - x13[0];
        r16[1] = x13[1] - w12[2];
        r16[2] = x13[2] + w12[1];
        r17[2] = y14[1] - w12[0];
        r17[0] = r17[0] + x13[1];
        r18[0] = r18[0] + x13[2];
        r18[1] = r18[1] + y14[1];
        r16[0] = r16[0] - z15;
        r17[1] = r17[1] - z15;
        r18[2] = r18[2] - y14[0];
        sw = I32(slot + 0x1C);
        pw = I32(slot + 0x2C);
        LD4(sc, slot + 0x10);
        LD4(tt, slot + 0x20);
        if (sw != 0) {
            for (i = 0; i < 3; i++) {
                r16[i] = r16[i] * sc[0];
                r17[i] = r17[i] * sc[1];
                r18[i] = r18[i] * sc[2];
            }
        }
        if (pw == 0) {
            P0[0] = 1.0f; P0[1] = 0.0f; P0[2] = 0.0f; P0[3] = 0.0f;
            P1[0] = 0.0f; P1[1] = 1.0f; P1[2] = 0.0f; P1[3] = 0.0f;
            P2[0] = 0.0f; P2[1] = 0.0f; P2[2] = 1.0f; P2[3] = 0.0f;
            P3[0] = 0.0f; P3[1] = 0.0f; P3[2] = 0.0f; P3[3] = 1.0f;
        } else {
            LD4(P0, (char *)pw);
            LD4(P1, (char *)pw + 0x10);
            LD4(P2, (char *)pw + 0x20);
            LD4(P3, (char *)pw + 0x30);
        }
        tt[0] = 0.0f + tt[0];
        tt[1] = 0.0f + tt[1];
        tt[2] = 0.0f + tt[2];
        tt[3] = 1.0f;
        for (i = 0; i < 4; i++) {
            o0[i] = (P0[i] * r16[0] + P1[i] * r16[1]) + P2[i] * r16[2];
            o1[i] = (P0[i] * r17[0] + P1[i] * r17[1]) + P2[i] * r17[2];
            o2[i] = (P0[i] * r18[0] + P1[i] * r18[1]) + P2[i] * r18[2];
            o3[i] = ((P0[i] * tt[0] + P1[i] * tt[1]) + P2[i] * tt[2]) + P3[i] * 1.0f;
        }
        PUT4(slot, o0);
        PUT4(slot + 0x10, o1);
        PUT4(slot + 0x20, o2);
        PUT4(slot + 0x30, o3);
    }

    /* post-scale list: rows of the joint matrix scaled by the (lerped) record */
    d = *(char **)(scratch + 0xC);
    while (d != 0) {
        float s[4], row[4];
        char *sl = sp + (((int)d - (int)(scratch + 0x10)) << 1);
        int i;

        if (two) {
            float a3[4], b3[4];

            LD4(a3, d);
            LD4(b3, d + 0x10);
            for (i = 0; i < 3; i++)
                s[i] = a3[i] * u + b3[i] * t;
        } else {
            LD4(s, d);
        }
        nxt = *(char **)(d + 0xC);
        for (i = 0; i < 3; i++) {
            LD4(row, sl + 0x10 * i);
            row[0] = row[0] * s[i];
            row[1] = row[1] * s[i];
            row[2] = row[2] * s[i];
            PUT4(sl + 0x10 * i, row);
        }
        d = nxt;
    }

    /* palette: joint matrix times the skeleton rows (w column of the skeleton never read) */
    {
        char *sk = sp + 0x1C00;

        slot = sp;
        for (j = 0; j < jc; j++, slot += 0x40, sk += 0x40) {
            float S0[4], S1[4], S2[4], S3[4], P0[4], P1[4], P2[4], P3[4];
            float o0[4], o1[4], o2[4], o3[4];
            int i;

            LD4(S0, sk);
            LD4(S1, sk + 0x10);
            LD4(S2, sk + 0x20);
            LD4(S3, sk + 0x30);
            LD4(P0, slot);
            LD4(P1, slot + 0x10);
            LD4(P2, slot + 0x20);
            LD4(P3, slot + 0x30);
            for (i = 0; i < 4; i++) {
                o3[i] = ((P0[i] * S3[0] + P1[i] * S3[1]) + P2[i] * S3[2]) + P3[i] * 1.0f;
                o0[i] = (P0[i] * S0[0] + P1[i] * S0[1]) + P2[i] * S0[2];
                o1[i] = (P0[i] * S1[0] + P1[i] * S1[1]) + P2[i] * S1[2];
                o2[i] = (P0[i] * S2[0] + P1[i] * S2[1]) + P2[i] * S2[2];
            }
            PUT4(slot, o0);
            PUT4(slot + 0x10, o1);
            PUT4(slot + 0x20, o2);
            PUT4(slot + 0x30, o3);
        }
    }
}
