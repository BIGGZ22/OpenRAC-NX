/* functional: medium. fcheck: unchecked. Hand-written second half of the optional extra pass (DrawOptionalExtraPass, after func_001F91B8 and Draw_BuildVu1Setup). Waits for the toSPR DMA (CHCR bit 8), then compacts the 8 light slots of scratchpad 0x2400 (each {a b c _, x y z r}; a slot is used when x or y is non-zero) in place to {a b c 1/r, x y z r}. Then for each of the D_0015F578 records at D_0015F57C (0x20 bytes: position, +0x14 alpha word from func_001F91B8, +0x18 TEX0) with the mesh pointer from D_0015F580: lit = (int)(256 * (1 - min(1, sum over lights within r of min(1, max(a,b,c) * 0.5) * (1 - dist/r)))) (VU0 vftoi0); when the low 16 bits of the alpha word are non-zero it patches TEX0 into the GS template D_001E0280 and copies its 3 quadwords to the packet pointer D_00161000, then per sub-mesh (count in the mesh word's high half, an alpha scale in its low half; each sub-mesh is {u32 n (even), n/2 * 12 bytes of u16 xyz pairs, 16-aligned n * 4 bytes of u16 uv}) builds a DMA cnt packet: the 5-quadword header D_001E0220 (n ORed into +0x10), the UNPACK tags D_00160F00 / D_00160F20 / D_00160F40 (n << 16 ORed into their last word) before the uv floats (h / 1024 - 32), the colours (0xFFFFFF | ((alpha * scale * lit) >> 16) << 24, with retail's sign-extension of the second word of each pair) and the positions (h / 1024 + record position - 32), each block zero-padded to 16 bytes, then a 0x15000000 | (0xC or 0xE with bit 31 of the alpha word), 0x11000000 tail, and sets the tag's qword count. Differences: retail polls bit 8 of the word at whatever address $t2 holds on entry (a register its C caller never sets; the intended register is the toSPR CHCR 0x1000D400, which this C polls); the 64-bit `bgtz` that rejects lights outside r also sees vf5.x (stale) when r^2 - dist^2 is exactly +0, taken as 0; vertex counts must be even and the record and sub-mesh counts non-zero (retail's loops would run away otherwise). Differential test: 120 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model), bit-exact. */

extern int D_0015F578;
extern unsigned char *D_0015F57C;
extern unsigned char **D_0015F580;
extern unsigned char *D_00161000;
extern unsigned int D_001E0280[];
extern unsigned int D_001E0220[];
extern unsigned int D_00160F00[];
extern unsigned int D_00160F20[];
extern unsigned int D_00160F40[];

typedef union {
    float f;
    unsigned int u;
} FU_1F8B6C;

/* VU0 vftoi0: truncate toward zero, saturate to 32 bits. */
static int ftoi0_1F8B6C(float x)
{
    FU_1F8B6C v;
    int e;
    unsigned int m;
    unsigned int r;

    v.f = x;
    e = (int)((v.u >> 23) & 0xFF) - 127;
    if (e < 0) {
        return 0;
    }
    if (e >= 31) {
        return (v.u >> 31) ? (int)0x80000000u : 0x7FFFFFFF;
    }
    m = (v.u & 0x7FFFFF) | 0x800000;
    r = e >= 23 ? m << (e - 23) : m >> (23 - e);
    return (v.u >> 31) ? -(int)r : (int)r;
}

/* VU0 vdiv 1/w: a zero divisor gives the largest float, signed as 1 ^ w. */
static float recip_1F8B6C(float w)
{
    FU_1F8B6C v;

    v.f = w;
    if ((v.u & 0x7F800000) == 0) {
        v.u = (v.u & 0x80000000) | 0x7F7FFFFF;
        return v.f;
    }
    return 1.0f / w;
}

static void qcopy_1F8B6C(unsigned char *d, const unsigned int *s)
{
    unsigned int *p = (unsigned int *)d;

    p[0] = s[0];
    p[1] = s[1];
    p[2] = s[2];
    p[3] = s[3];
}

/* Zero words up to and including the next 16-byte boundary (retail stores in the branch delay slot). */
static unsigned char *pad_1F8B6C(unsigned char *q)
{
    for (;;) {
        int aligned = ((unsigned long)q & 15) == 0;

        *(unsigned int *)q = 0;
        if (aligned) {
            return q;
        }
        q += 4;
    }
}

void func_001F8B6C(void)
{
    unsigned char *sp = (unsigned char *)0x70000000;
    unsigned char *in;
    unsigned char *out;
    int nl;
    int n;
    unsigned char *rec;
    unsigned char **pp;

    while (*(volatile unsigned int *)0x1000D400 & 0x100) {
    }

    /* Compact the used light slots. */
    nl = 0;
    in = sp + 0x2400;
    out = sp + 0x2400;
    while (in != sp + 0x2500) {
        unsigned int *w = (unsigned int *)(in + 0x10);
        float v1[4];
        float v2[4];
        int k;

        in += 0x20;
        if (w[0] == 0 && w[1] == 0) {
            continue;
        }
        for (k = 0; k < 4; k++) {
            v1[k] = ((float *)(in - 0x10))[k];
            v2[k] = ((float *)(in - 0x20))[k];
        }
        v2[3] = recip_1F8B6C(v1[3]);
        for (k = 0; k < 4; k++) {
            ((float *)(out + 0x10))[k] = v1[k];
            ((float *)out)[k] = v2[k];
        }
        nl++;
        out += 0x20;
    }

    n = D_0015F578;
    rec = D_0015F57C;
    pp = D_0015F580;
    do {
        unsigned char *mesh = *pp;
        float *r = (float *)rec;
        float sum = 0.0f;
        unsigned int alpha;
        int i;

        for (i = 0; i < nl; i++) {
            float *par = (float *)(sp + 0x2400 + i * 0x20);
            float *pos = par + 4;
            float dx = pos[0] - r[0];
            float dy = pos[1] - r[1];
            float dz = pos[2] - r[2];
            float d2 = (dy * dy + dx * dx) + dz * dz;
            float t = d2 - pos[3] * pos[3];
            float m;
            float f;

            if (t > 0.0f) {
                continue;
            }
            f = 1.0f - par[3] * sqrtf(d2);
            m = par[0];
            if (par[1] > m) {
                m = par[1];
            }
            if (par[2] > m) {
                m = par[2];
            }
            m = m * 0.5f;
            if (m > 1.0f) {
                m = 1.0f;
            }
            sum = sum + m * f;
        }
        if (sum > 1.0f) {
            sum = 1.0f;
        }

        alpha = *(unsigned int *)(rec + 0x14);
        if ((alpha & 0xFFFF) != 0) {
            unsigned int lit = (unsigned int)ftoi0_1F8B6C((1.0f - sum) * 256.0f);
            unsigned int kind = ((alpha >> 31) << 1) + 0xC;
            unsigned int hdr = *(unsigned int *)mesh;
            unsigned int a6 = ((alpha & 0xFFFF) * (hdr & 0xFFFF) * lit) >> 16;
            unsigned int nsub = hdr >> 16;
            unsigned char *mp = mesh + 8;
            float bx = r[0] - 32.0f;
            float by = r[1] - 32.0f;
            float bz = r[2] - 32.0f;
            unsigned char *q;

            D_001E0280[8] = *(unsigned int *)(rec + 0x18);
            D_001E0280[9] = *(unsigned int *)(rec + 0x1C);
            q = D_00161000;
            qcopy_1F8B6C(q, D_001E0280);
            qcopy_1F8B6C(q + 0x10, D_001E0280 + 4);
            qcopy_1F8B6C(q + 0x20, D_001E0280 + 8);
            D_00161000 = q + 0x30;

            do {
                unsigned int nv;
                unsigned char *posp;
                unsigned char *uvp;
                unsigned char *pk;
                unsigned int c;
                unsigned int clo, chi;

                mp = (unsigned char *)(((unsigned long)mp + 15) & ~15ul);
                nv = *(unsigned int *)mp;
                mp += 4;
                posp = mp;
                mp += (unsigned int)((int)nv >> 1) * 12u;
                mp = (unsigned char *)(((unsigned long)mp + 15) & ~15ul);
                uvp = mp;
                mp += nv * 4;

                pk = D_00161000;
                for (i = 0; i < 5; i++) {
                    qcopy_1F8B6C(pk + i * 0x10, D_001E0220 + i * 4);
                }
                *(unsigned int *)(pk + 0x10) |= nv;
                q = pk + 0x50;
                qcopy_1F8B6C(q, D_00160F00);
                *(unsigned int *)(q + 0xC) |= nv << 16;
                q += 0x10;

                c = nv;
                do {
                    unsigned short *h = (unsigned short *)uvp;
                    float *o = (float *)q;

                    for (i = 0; i < 4; i++) {
                        o[i] = (float)(int)((unsigned int)h[i] << 2) * (1.0f / 4096.0f) - 32.0f;
                    }
                    uvp += 8;
                    q += 0x10;
                    c -= 2;
                } while (c != 0);
                q = pad_1F8B6C(q);

                qcopy_1F8B6C(q, D_00160F20);
                *(unsigned int *)(q + 0xC) |= nv << 16;
                q += 0x10;
                clo = (a6 << 24) | 0xFFFFFF;
                chi = (clo & 0x80000000u) ? 0xFFFFFFFFu : clo;
                c = nv;
                do {
                    ((unsigned int *)q)[0] = clo;
                    ((unsigned int *)q)[1] = chi;
                    q += 8;
                    c -= 2;
                } while (c != 0);
                q = pad_1F8B6C(q);

                qcopy_1F8B6C(q, D_00160F40);
                *(unsigned int *)(q + 0xC) |= nv << 16;
                q += 0x10;
                c = nv;
                do {
                    unsigned short *h = (unsigned short *)posp;
                    float *o = (float *)q;

                    o[0] = (float)(int)((unsigned int)h[0] << 2) * (1.0f / 4096.0f) + bx;
                    o[1] = (float)(int)((unsigned int)h[1] << 2) * (1.0f / 4096.0f) + by;
                    o[2] = (float)(int)((unsigned int)h[2] << 2) * (1.0f / 4096.0f) + bz;
                    o[3] = (float)(int)((unsigned int)h[3] << 2) * (1.0f / 4096.0f) + bx;
                    o[4] = (float)(int)((unsigned int)h[4] << 2) * (1.0f / 4096.0f) + by;
                    o[5] = (float)(int)((unsigned int)h[5] << 2) * (1.0f / 4096.0f) + bz;
                    posp += 12;
                    q += 0x18;
                    c -= 2;
                } while (c != 0);
                q = pad_1F8B6C(q);

                ((unsigned int *)q)[0] = 0x15000000u | kind;
                ((unsigned int *)q)[1] = 0x11000000u;
                ((unsigned int *)q)[2] = 0;
                ((unsigned int *)q)[3] = 0;
                q += 0x10;
                *(unsigned int *)pk |= ((unsigned int)(q - pk) >> 4) - 1;
                D_00161000 = q;
            } while (--nsub != 0);
        }
        rec += 0x20;
        pp++;
    } while (--n != 0);
}
