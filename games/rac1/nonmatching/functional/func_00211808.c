/* functional: high. fcheck: unchecked. Moby joint pose evaluation (handwritten VU0 macro code, names.h: moby_anim_eval_chain; called only from the handwritten func_00211548/func_002116A0, but with a0 = moby, a1 = joint mask in the ABI registers). Fetches the class bind data (moby->pclass+0x18, pclass[8] joints) and the current/previous animation keys (moby+0x68/+0x6C) into the scratchpad at 0x70002000 with one DMA chain (tags written to D_001B3180, -1 completion sentinels), then fills the joint slots (0x70000000 + 0x40*j): bind translation to +0x20/+0x30, flagged unsigned 3.12 scale entries to +0/+0x10 and signed translation entries to +0x20/+0x30 (linked lists through +0xC / +0x3C, heads at 0x70002000/4), lerped by moby->anim_time (0x54); s1.15 quaternions two joints at a time (count mask[0x7F]), lerped, renormalised with the sign flip on a negative dot unless the frame/seq bytes 0x50..0x53 say the keys match; t == 0 takes the current key only. Then the pose layers at moby+0x60 (DMA'd records blended into their slots by the layer weight), the manipulators at moby+0x64 (quaternion product / scale product / translation sum, or a weighted blend) and finally, for each joint with a nonzero mask byte (the mask ends at index (s8)mask[0x7F]-1), the 4x4 matrix: rotation from the quaternion, rows scaled when +0x1C is set, translation, times the parent's matrix at the slot address in +0x2C. Each DMA is a copy made where retail starts it (the port has no DMA controller and the sentinel waits would spin); the DMA registers are not written. Retail quirks kept: slot +0x30 gets the bind w reduced to its sign bit, bind rows are copied four joints at a time, the matrix pass checks its end only on converted joints. Not reproduced: VU0 overflow clamping (an overflowing float is the largest float on the VU, inf here) and VU rounding (the lane arithmetic is plain IEEE in retail's operation order). Differential test: 121 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model, plus a model of DMA channel 9 doing the transfers at once) to 1e-5 relative, every retail instruction executed (t = 0 / 1 / random, matching and differing keys, flagged and unflagged entries, layers, both manipulator kinds, parents, masked joints, no joints); the test skeletons keep layer/manipulator targets within the keyed joints and converted joints' parents converted, as real data does. */
extern float sqrtf(float);
extern u8 D_001B3180[];

#define SPR_211808 ((u8 *)0x70000000)
#define W_211808(p, o) (*(u32 *)((u8 *)(p) + (o)))
#define F_211808(p, o) (*(float *)((u8 *)(p) + (o)))
#define H_211808(p, o) (*(u16 *)((u8 *)(p) + (o)))
#define S_211808(p, o) (*(s16 *)((u8 *)(p) + (o)))

/* A DMA transfer of qwc quadwords to the scratchpad, done at once (the port has no DMA controller). */
static void spr_copy_211808(u8 *dst, u8 *src, int qwc)
{
    int i;

    for (i = 0; i < qwc * 4; i++)
        ((u32 *)dst)[i] = ((u32 *)src)[i];
}

/* vrsqrt Q, vf0w, x: 1/sqrt(x), the largest float for x == 0 */
static float rsq_211808(float x)
{
    if (x == 0.0f)
        return 3.40282347e38f;
    return 1.0f / sqrtf(x);
}

static int neg_211808(float x)
{
    union { float f; u32 u; } v;

    v.f = x;
    return v.u >> 31;
}

/* ((y + x) + z) + w, the order of the vaddax/vmaddaz/vmaddw chain */
static float dot_yx_211808(const float *v)
{
    return ((v[1] + v[0]) + v[2]) + v[3];
}

static float dot_xy_211808(const float *v)
{
    return ((v[0] + v[1]) + v[2]) + v[3];
}

/* a quaternion blend a*s + b*t, renormalised; the sign of b is flipped when a.b < 0 */
static void qblend_211808(float *out, const float *a0, const float *b0, float s, float t)
{
    float a[4], b[4], p[4], d[4], sq[4], q;
    int i;

    for (i = 0; i < 4; i++) {
        a[i] = a0[i] * s;
        b[i] = b0[i] * t;
    }
    for (i = 0; i < 4; i++) {
        d[i] = a[i] + b[i];
        p[i] = a[i] * b[i];
    }
    if (neg_211808(dot_yx_211808(p))) {
        for (i = 0; i < 4; i++)
            d[i] = a[i] - b[i];
    }
    for (i = 0; i < 4; i++)
        sq[i] = d[i] * d[i];
    q = rsq_211808(dot_xy_211808(sq));
    for (i = 0; i < 4; i++)
        out[i] = d[i] * q;
}

void func_00211808(void *moby, u8 *mask)
{
    u8 *m = (u8 *)moby;
    u8 *spr = SPR_211808;
    u8 *cls = (u8 *)W_211808(m, 0x24);
    u8 *cur = (u8 *)W_211808(m, 0x68);
    u8 *prv = (u8 *)W_211808(m, 0x6C);
    u32 tbits = W_211808(m, 0x54);
    int nj = cls[8];
    int n1, n2, cA, cB, pA, pB, k, i, j, cnt, same, d;
    u8 *b1, *b2, *e, *ent, *en, *slot, *tailA, *tailB, *q1, *q2, *p;
    u32 old;
    float t, s;

    if (nj == 0)
        return;

    /* DMA chain to the scratchpad at 0x2000: class bind data (nj qw), current key, previous key */
    H_211808(D_001B3180, 0x00) = nj;
    H_211808(D_001B3180, 0x02) = 0x3000;
    W_211808(D_001B3180, 0x04) = W_211808(cls, 0x18);
    n1 = S_211808(cur, 6);
    H_211808(D_001B3180, 0x12) = 0x3000;
    H_211808(D_001B3180, 0x10) = n1;
    W_211808(D_001B3180, 0x14) = (u32)(cur + 0x10);
    n2 = H_211808(prv, 6);
    W_211808(D_001B3180, 0x24) = (u32)(prv + 0x10);
    W_211808(D_001B3180, 0x20) = n2;
    if (tbits == 0)
        H_211808(D_001B3180, 0x12) = 0;   /* refe: the previous key is not fetched */

    b1 = spr + 0x2000 + nj * 16;
    b2 = b1 + n1 * 16;
    e = b2 + n2 * 16;
    W_211808(b1, -4) = 0xFFFFFFFF;   /* completion sentinels */
    W_211808(b2, -4) = 0xFFFFFFFF;
    W_211808(e, -4) = 0xFFFFFFFF;
    spr_copy_211808(spr + 0x2000, (u8 *)W_211808(cls, 0x18), nj);
    spr_copy_211808(b1, cur + 0x10, n1 & 0xFFFF);
    if (tbits != 0)
        spr_copy_211808(b2, prv + 0x10, n2);

    t = *(float *)&tbits;
    s = 1.0f - t;

    /* slots 0..nj-1: +0 and +0x10 = (1, 1, 1, 0) */
    for (i = 0; i < nj; i++) {
        for (j = 0; j < 2; j++) {
            F_211808(spr + i * 64, j * 16 + 0) = 1.0f;
            F_211808(spr + i * 64, j * 16 + 4) = 1.0f;
            F_211808(spr + i * 64, j * 16 + 8) = 1.0f;
            F_211808(spr + i * 64, j * 16 + 12) = 0.0f;
        }
    }

    d = ((m[0x51] - m[0x50]) & 0xFFFE) + (m[0x52] - m[0x53]);
    same = (d == 0);

    /* bind translations to slot+0x20, and slot+0x30 with w reduced to its sign; four joints at a time */
    k = 0;
    do {
        for (j = 0; j < 4; j++) {
            u8 *src = spr + 0x2000 + (k + j) * 16;
            u8 *dst = spr + (k + j) * 64;
            u32 w0 = W_211808(src, 0), w1 = W_211808(src, 4), w2 = W_211808(src, 8), w3 = W_211808(src, 12);

            W_211808(dst, 0x20) = w0;
            W_211808(dst, 0x24) = w1;
            W_211808(dst, 0x28) = w2;
            W_211808(dst, 0x2C) = w3;
            W_211808(dst, 0x30) = w0;
            W_211808(dst, 0x34) = w1;
            W_211808(dst, 0x38) = w2;
            W_211808(dst, 0x3C) = w3 & 0x80000000;
        }
        k += 4;
    } while (nj - k > 0);

    W_211808(spr, 0x2000) = 0;   /* list heads A (+0) and B (+4) */
    W_211808(spr, 0x2004) = 0;
    W_211808(spr, 0x2008) = 0;
    W_211808(spr, 0x200C) = 0;

    cA = S_211808(cur, 0xA);
    cB = S_211808(cur, 0xE);
    pA = S_211808(prv, 0xA);
    pB = S_211808(prv, 0xE);
    ent = b1 + nj * 8;
    cnt = mask[0x7F];

    if (tbits != 0) {
        tailA = spr + 0x1FF4;   /* its +0xC is head A */
        tailB = spr + 0x1FC8;   /* its +0x3C is head B */

        /* current key, list A: flagged unsigned 3.12 vectors to slot+0, flag to +0x1C, linked by +0xC */
        while (cA != 0) {
            en = ent;
            ent += 8;
            cA--;
            if (!(en[7] & 0x80))
                continue;
            slot = spr + en[6] * 64;
            W_211808(tailA, 0xC) = (u32)slot;
            tailA = slot;
            W_211808(slot, 0x1C) = (en[7] >> 6) - 1;
            F_211808(slot, 0) = (float)(H_211808(en, 0) << 3) / 32768.0f;
            F_211808(slot, 4) = (float)(H_211808(en, 2) << 3) / 32768.0f;
            F_211808(slot, 8) = (float)(H_211808(en, 4) << 3) / 32768.0f;
            W_211808(slot, 0xC) = 0;
        }
        /* current key, list B: signed integer vectors to slot+0x20 (w kept), linked by +0x3C */
        while (cB != 0) {
            en = ent;
            ent += 8;
            cB--;
            slot = spr + en[6] * 64;
            W_211808(tailB, 0x3C) = (u32)slot;
            tailB = slot;
            F_211808(slot, 0x20) = (float)S_211808(en, 0);
            F_211808(slot, 0x24) = (float)S_211808(en, 2);
            F_211808(slot, 0x28) = (float)S_211808(en, 4);
        }
        W_211808(tailB, 0x3C) = 0;

        /* previous key, list A: to slot+0x10; appended to list A when the slot's +0xC is 0 */
        ent = b2 + nj * 8;
        while (pA != 0) {
            en = ent;
            ent += 8;
            pA--;
            slot = spr + en[6] * 64;
            old = W_211808(slot, 0xC);
            if (!(en[7] & 0x80))
                continue;
            F_211808(slot, 0x10) = (float)(H_211808(en, 0) << 3) / 32768.0f;
            F_211808(slot, 0x14) = (float)(H_211808(en, 2) << 3) / 32768.0f;
            F_211808(slot, 0x18) = (float)(H_211808(en, 4) << 3) / 32768.0f;
            W_211808(slot, 0x1C) = (en[7] >> 6) - 1;
            if (old == 0) {
                W_211808(tailA, 0xC) = (u32)slot;
                tailA = slot;
                W_211808(slot, 0xC) = 0;
            }
        }
        /* previous key, list B: to slot+0x30; appended to list B when the slot's +0x3C is 0 */
        while (pB != 0) {
            en = ent;
            ent += 8;
            pB--;
            slot = spr + en[6] * 64;
            old = W_211808(slot, 0x3C);
            F_211808(slot, 0x30) = (float)S_211808(en, 0);
            F_211808(slot, 0x34) = (float)S_211808(en, 2);
            F_211808(slot, 0x38) = (float)S_211808(en, 4);
            if (old == 0) {
                W_211808(tailB, 0x3C) = (u32)slot;
                tailB = slot;
                W_211808(slot, 0x3C) = 0;
            }
        }

        /* lerp the lists by t: A into slot+0x10, B into slot+0x20 */
        for (p = (u8 *)W_211808(spr, 0x2000); p != 0; p = (u8 *)W_211808(p, 0xC)) {
            for (i = 0; i < 3; i++)
                F_211808(p, 0x10 + i * 4) = F_211808(p, i * 4) * s + F_211808(p, 0x10 + i * 4) * t;
        }
        for (p = (u8 *)W_211808(spr, 0x2004); p != 0; p = (u8 *)W_211808(p, 0x3C)) {
            for (i = 0; i < 3; i++)
                F_211808(p, 0x20 + i * 4) = F_211808(p, 0x20 + i * 4) * s + F_211808(p, 0x30 + i * 4) * t;
        }

        /* rotations, two joints per pass: s1.15 quaternions of both keys, blended */
        q1 = b1;
        q2 = b2;
        slot = spr;
        while (cnt > 0) {
            cnt -= 2;
            for (j = 0; j < 2; j++) {
                float a[4], b[4], r[4];

                for (i = 0; i < 4; i++) {
                    a[i] = (float)S_211808(q1, j * 8 + i * 2) / 32768.0f;
                    b[i] = (float)S_211808(q2, j * 8 + i * 2) / 32768.0f;
                }
                if (same) {
                    for (i = 0; i < 4; i++)
                        r[i] = a[i] * s + b[i] * t;
                } else {
                    float pr[4], sq[4], q;

                    for (i = 0; i < 4; i++)
                        pr[i] = a[i] * b[i];
                    if (neg_211808(dot_yx_211808(pr))) {
                        for (i = 0; i < 4; i++)
                            r[i] = a[i] * s - b[i] * t;
                    } else {
                        for (i = 0; i < 4; i++)
                            r[i] = a[i] * s + b[i] * t;
                    }
                    for (i = 0; i < 4; i++)
                        sq[i] = r[i] * r[i];
                    q = rsq_211808(dot_xy_211808(sq));
                    for (i = 0; i < 4; i++)
                        r[i] = r[i] * q;
                }
                for (i = 0; i < 4; i++)
                    F_211808(slot, j * 0x40 + i * 4) = r[i];
            }
            q1 += 16;
            q2 += 16;
            slot += 0x80;
        }
    } else {
        /* t == 0: the current key only */
        while (cA != 0) {
            en = ent;
            ent += 8;
            cA--;
            if (!(en[7] & 0x80))
                continue;
            slot = spr + en[6] * 64;
            F_211808(slot, 0x10) = (float)(H_211808(en, 0) << 3) / 32768.0f;
            F_211808(slot, 0x14) = (float)(H_211808(en, 2) << 3) / 32768.0f;
            F_211808(slot, 0x18) = (float)(H_211808(en, 4) << 3) / 32768.0f;
            W_211808(slot, 0x1C) = (en[7] >> 6) - 1;
        }
        while (cB != 0) {
            en = ent;
            ent += 8;
            cB--;
            slot = spr + en[6] * 64;
            F_211808(slot, 0x30) = 0.0f;
            F_211808(slot, 0x34) = 0.0f;
            F_211808(slot, 0x38) = 0.0f;
            F_211808(slot, 0x3C) = 1.0f;
            F_211808(slot, 0x20) = (float)S_211808(en, 0);
            F_211808(slot, 0x24) = (float)S_211808(en, 2);
            F_211808(slot, 0x28) = (float)S_211808(en, 4);
        }
        q1 = b1;
        slot = spr;
        while (cnt > 0) {
            cnt -= 2;
            for (j = 0; j < 2; j++)
                for (i = 0; i < 4; i++)
                    F_211808(slot, j * 0x40 + i * 4) = (float)S_211808(q1, j * 8 + i * 2) / 32768.0f;
            q1 += 16;
            slot += 0x80;
        }
    }

    /* pose layers (moby+0x60): records of 3 qw {quaternion, scale, translation | +0x2C slot address},
       blended into their slots by the layer weight */
    p = (u8 *)W_211808(m, 0x60);
    while (p != 0) {
        int n = H_211808(p, 2);
        float t2 = F_211808(p, 8), s2;
        u8 *r = spr + 0x2000, *end = spr + 0x2000 + n * 3 * 16;

        spr_copy_211808(spr + 0x2000, (u8 *)W_211808(p, 0x10), n * 3);
        p = (u8 *)W_211808(p, 0x1C);
        s2 = 1.0f - t2;
        do {
            float v11[3], v12[3], qq[4];

            slot = (u8 *)W_211808(r, 0x2C);
            qblend_211808(qq, (float *)slot, (float *)r, s2, t2);
            for (i = 0; i < 3; i++) {
                v11[i] = F_211808(slot, 0x10 + i * 4) * s2 + F_211808(r, 0x10 + i * 4) * t2;
                v12[i] = F_211808(slot, 0x20 + i * 4) * s2 + F_211808(r, 0x20 + i * 4) * t2;
            }
            for (i = 0; i < 3; i++) {
                F_211808(slot, 0x10 + i * 4) = v11[i];
                F_211808(slot, 0x20 + i * 4) = v12[i];
            }
            if (W_211808(slot, 0x1C) == 0)
                W_211808(slot, 0x1C) = W_211808(r, 0x1C);
            for (i = 0; i < 4; i++)
                F_211808(slot, i * 4) = qq[i];
            r += 0x30;
        } while (r != end);
    }

    /* joint manipulators (moby+0x64): byte 3 == 0 multiplies (rotation product, scale product,
       translation sum), otherwise blends by the weight at +0xC */
    p = (u8 *)W_211808(m, 0x64);
    while (p != 0) {
        float v6[4], v7[4], v8[3], v10[3];

        slot = (u8 *)W_211808(p, 4);
        for (i = 0; i < 4; i++) {
            v6[i] = F_211808(slot, i * 4);
            v7[i] = F_211808(p, 0x10 + i * 4);
        }
        if (p[3] == 0) {
            float w, pr[3], a[3], b[3], ac[3], cr[3], d3;

            w = v7[3] * v6[3];
            for (i = 0; i < 3; i++) {
                pr[i] = v7[i] * v6[i];
                a[i] = v7[i] * v6[3];
                b[i] = v6[i] * v7[3];
            }
            ac[0] = v6[1] * v7[2];
            ac[1] = v6[2] * v7[0];
            ac[2] = v6[0] * v7[1];
            cr[0] = ac[0] - v7[1] * v6[2];
            cr[1] = ac[1] - v7[2] * v6[0];
            cr[2] = ac[2] - v7[0] * v6[1];
            d3 = (pr[0] + pr[1]) + pr[2];
            for (i = 0; i < 3; i++)
                v6[i] = (a[i] + b[i]) + cr[i];
            v6[3] = w - d3;
            for (i = 0; i < 3; i++) {
                v8[i] = F_211808(slot, 0x10 + i * 4) * F_211808(p, 0x20 + i * 4);
                v10[i] = F_211808(slot, 0x20 + i * 4) + F_211808(p, 0x30 + i * 4);
            }
        } else {
            float wt = F_211808(p, 0xC);
            float sw = 1.0f - wt;

            qblend_211808(v6, v6, v7, sw, wt);
            for (i = 0; i < 3; i++) {
                v8[i] = F_211808(slot, 0x10 + i * 4) * sw + F_211808(p, 0x20 + i * 4) * wt;
                v10[i] = F_211808(slot, 0x20 + i * 4) * sw + F_211808(p, 0x30 + i * 4) * wt;
            }
        }
        p = (u8 *)W_211808(p, 8);
        for (i = 0; i < 4; i++)
            F_211808(slot, i * 4) = v6[i];
        for (i = 0; i < 3; i++) {
            F_211808(slot, 0x10 + i * 4) = v8[i];
            F_211808(slot, 0x20 + i * 4) = v10[i];
        }
        W_211808(slot, 0x1C) = 1;
    }

    /* joint matrices for the joints with a nonzero mask byte, in slot order (mask ends with a
       nonzero byte at index (s8)mask[0x7F] - 1): rotation from the quaternion, rows scaled when
       +0x1C is set, translation +0x20, times the parent matrix at the slot address in +0x2C */
    cnt = (s8)mask[0x7F];
    p = spr;
    e = spr + cnt * 64;
    i = 0;
    do {
        float x, y, z, w, X, Y, Z, c12[3], c13[3], c14[3], c15, r0[3], r1[3], r2[3], tr[3];
        float P[4][4], o[4][4];
        u8 *par;
        int f;

        do {
            f = (s8)mask[i++];
            slot = p;
            p += 0x40;
        } while (f == 0);

        x = F_211808(slot, 0);
        y = F_211808(slot, 4);
        z = F_211808(slot, 8);
        w = F_211808(slot, 12);
        X = x + x;
        Y = y + y;
        Z = z + z;
        c12[0] = X * w; c12[1] = Y * w; c12[2] = Z * w;
        c13[0] = X * x; c13[1] = Y * x; c13[2] = Z * x;
        c14[1] = Y * y; c14[2] = Z * y;
        c15 = Z * z;
        r1[0] = 0.0f + c12[2];
        r2[0] = 0.0f - c12[1];
        r2[1] = 0.0f + c12[0];
        r0[0] = 1.0f - c14[1];
        r1[1] = 1.0f - c13[0];
        r2[2] = 1.0f - c13[0];
        r0[1] = c13[1] - c12[2];
        r0[2] = c13[2] + c12[1];
        r1[2] = c14[2] - c12[0];
        r1[0] = r1[0] + c13[1];
        r2[0] = r2[0] + c13[2];
        r2[1] = r2[1] + c14[2];
        r0[0] = r0[0] - c15;
        r1[1] = r1[1] - c15;
        r2[2] = r2[2] - c14[1];

        if (W_211808(slot, 0x1C) != 0) {
            for (j = 0; j < 3; j++) {
                r0[j] = r0[j] * F_211808(slot, 0x10);
                r1[j] = r1[j] * F_211808(slot, 0x14);
                r2[j] = r2[j] * F_211808(slot, 0x18);
            }
        }
        par = (u8 *)W_211808(slot, 0x2C);
        for (j = 0; j < 4; j++)
            for (k = 0; k < 4; k++)
                P[j][k] = par != 0 ? F_211808(par, j * 16 + k * 4) : (j == k ? 1.0f : 0.0f);
        for (j = 0; j < 3; j++)
            tr[j] = 0.0f + F_211808(slot, 0x20 + j * 4);

        for (k = 0; k < 4; k++) {
            o[0][k] = (P[0][k] * r0[0] + P[1][k] * r0[1]) + P[2][k] * r0[2];
            o[1][k] = (P[0][k] * r1[0] + P[1][k] * r1[1]) + P[2][k] * r1[2];
            o[2][k] = (P[0][k] * r2[0] + P[1][k] * r2[1]) + P[2][k] * r2[2];
            o[3][k] = ((P[0][k] * tr[0] + P[1][k] * tr[1]) + P[2][k] * tr[2]) + P[3][k];
        }
        for (j = 0; j < 4; j++)
            for (k = 0; k < 4; k++)
                F_211808(slot, j * 16 + k * 4) = o[j][k];
    } while (p != e);
}
