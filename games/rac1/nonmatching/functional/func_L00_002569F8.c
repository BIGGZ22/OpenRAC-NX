/* functional: medium. fcheck: unchecked (not listed for the port, so OpenRAC tools/fcheck_gate.py gave no verdict). MobyProc, the moby draw-list builder, translated instruction by instruction from the assembly: the VU0 macro-mode arithmetic is done on a float register file vf[32][4] with an accumulator and Q (lane masks, broadcasts, ftoi/itof, rsqrt as 1/sqrt(|x|), divide by zero giving the largest float), the 128-bit register halves ($21, $22, $29, $15, $14) are kept as separate 64-bit variables, and every branch delay slot is applied where retail applies it. Retail's DMA transfers to and from the scratchpad (channels 8 and 9) are copies made at the point retail starts them, and the waits on their control registers are dropped (the port has no DMA controller and they would spin forever). The tail code of the following catalogue entry func_L00_00257848 (the pass over the queued fading mobys, the billboard size test, the light list push and the shadow packet loop) is part of this function, so it is translated inline here as goto targets; the entry is not a function of its own. Not run against retail (hand-checked against the assembly only); the words the assembly leaves undefined (vf lanes never written, vf12..vf14 w, the stale word at +4 of a queue entry) are taken as zero or as the scratchpad content. */
extern float sqrtf(float);

#define M_X 1
#define M_Y 2
#define M_Z 4
#define M_W 8
#define M_XY 3
#define M_XZ 5
#define M_XYZ 7
#define M_XYZW 15

enum { O_ADD, O_SUB, O_MUL, O_MAX, O_MADD, O_ADDA, O_MULA, O_MADDA };

#define SPB 0x70000000u
#define LW(a) ((long)*(int *)(unsigned int)(a))
#define LWU(a) ((long)*(unsigned int *)(unsigned int)(a))
#define LH(a) ((long)*(short *)(unsigned int)(a))
#define LHU(a) ((long)*(unsigned short *)(unsigned int)(a))
#define LB(a) ((long)*(signed char *)(unsigned int)(a))
#define LBU(a) ((long)*(unsigned char *)(unsigned int)(a))
#define LD(a) (*(long *)(unsigned int)(a))
#define SW(a, v) (*(int *)(unsigned int)(a) = (int)(v))
#define SH(a, v) (*(short *)(unsigned int)(a) = (short)(v))
#define SB(a, v) (*(char *)(unsigned int)(a) = (char)(v))
#define SD(a, v) (*(long *)(unsigned int)(a) = (long)(v))
#define U32(x) ((unsigned int)(x))
#define S32(x) ((long)(int)(x))

static unsigned int fb_2569F8(float f)
{
    union { float f; unsigned int u; } x;
    x.f = f;
    return x.u;
}

static float bf_2569F8(unsigned int u)
{
    union { float f; unsigned int u; } x;
    x.u = u;
    return x.f;
}

/* quadword copy of n quads, source and destination as game addresses */
static void qcopy_2569F8(unsigned int dst, unsigned int src, int n)
{
    int *d = (int *)dst;
    int *s = (int *)src;
    int i;

    for (i = 0; i < n * 4; i++) {
        d[i] = s[i];
    }
}

static void lqv_2569F8(float *v, unsigned int a)
{
    int i;

    for (i = 0; i < 4; i++) {
        v[i] = bf_2569F8(*(unsigned int *)(a + 4 * i));
    }
}

static void sqv_2569F8(unsigned int a, const float *v)
{
    int i;

    for (i = 0; i < 4; i++) {
        *(unsigned int *)(a + 4 * i) = fb_2569F8(v[i]);
    }
}

/* One VU0 arithmetic instruction: d = s op t on the lanes in mask m. tb < 0 takes t lane by lane,
 * tb 0..3 broadcasts t[tb]. The *A forms write the accumulator, MADD adds it. */
static void vu_2569F8(float vf[32][4], float *acc, int op, int d, int s, const float *t, int tb, int m)
{
    float bc = tb >= 0 ? t[tb] : 0.0f;
    int i;

    for (i = 0; i < 4; i++) {
        float a, b, p;

        if (!(m & (1 << i))) {
            continue;
        }
        a = vf[s][i];
        b = tb >= 0 ? bc : t[i];
        switch (op) {
        case O_ADD: vf[d][i] = a + b; break;
        case O_SUB: vf[d][i] = a - b; break;
        case O_MUL: vf[d][i] = a * b; break;
        case O_MAX: vf[d][i] = a > b ? a : b; break;
        case O_MADD: p = a * b; vf[d][i] = acc[i] + p; break;
        case O_ADDA: acc[i] = a + b; break;
        case O_MULA: acc[i] = a * b; break;
        case O_MADDA: p = a * b; acc[i] = acc[i] + p; break;
        }
    }
}

/* vftoi0 / vitof0 / vitof12 on the lanes in m */
static void vftoi0_2569F8(float vf[32][4], int d, int s, int m)
{
    int i;

    for (i = 0; i < 4; i++) {
        float x;
        int v;

        if (!(m & (1 << i))) {
            continue;
        }
        x = vf[s][i];
        if (x >= 2147483648.0f) {
            v = 0x7FFFFFFF;
        } else if (x <= -2147483648.0f) {
            v = (int)0x80000000;
        } else {
            v = (int)x;
        }
        vf[d][i] = bf_2569F8((unsigned int)v);
    }
}

static void vitof_2569F8(float vf[32][4], int d, int s, float div, int m)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (m & (1 << i)) {
            vf[d][i] = (float)(int)fb_2569F8(vf[s][i]) / div;
        }
    }
}

/* the low / high 64 bits of a vf register as a GPR sees them after qmfc2 */
static long qlo_2569F8(const float *v)
{
    return (long)(((unsigned long)fb_2569F8(v[1]) << 32) | (unsigned long)fb_2569F8(v[0]));
}

static long qhi_2569F8(const float *v)
{
    return (long)(((unsigned long)fb_2569F8(v[3]) << 32) | (unsigned long)fb_2569F8(v[2]));
}

/* the VU divide unit: no NaN, no infinity */
static float vdiv_2569F8(float a, float b)
{
    if (b == 0.0f) {
        return ((fb_2569F8(a) ^ fb_2569F8(b)) & 0x80000000u) ? -3.4028234e38f : 3.4028234e38f;
    }
    return a / b;
}

static float vrsqrt_2569F8(float a, float b)
{
    float r = b < 0.0f ? -b : b;

    if (r == 0.0f) {
        return (fb_2569F8(a) & 0x80000000u) ? -3.4028234e38f : 3.4028234e38f;
    }
    return a / sqrtf(r);
}

static float vsqrt_2569F8(float b)
{
    return sqrtf(b < 0.0f ? -b : b);
}

#define VV(op, d, s, t, m) vu_2569F8(vf, acc, op, d, s, vf[t], -1, m)
#define VB(op, d, s, t, bc, m) vu_2569F8(vf, acc, op, d, s, vf[t], bc, m)
#define VQ(op, d, s, m) vu_2569F8(vf, acc, op, d, s, &Q, 0, m)
#define LQV(d, a) lqv_2569F8(vf[d], U32(a))
#define SQV(a, s) sqv_2569F8(U32(a), vf[s])
/* ACC = a*b-row combination: acc = r0*x + r1*y + r2*z (+ r3*w), result into d, lanes m */
#define MAT3(d, r0, r1, r2, src, m) do { \
        VB(O_MULA, 0, r0, src, 0, m); \
        VB(O_MADDA, 0, r1, src, 1, m); \
        VB(O_MADD, d, r2, src, 2, m); \
    } while (0)
#define MAT4(d, r0, r1, r2, r3, src, m) do { \
        VB(O_MULA, 0, r0, src, 0, m); \
        VB(O_MADDA, 0, r1, src, 1, m); \
        VB(O_MADDA, 0, r2, src, 2, m); \
        VB(O_MADD, d, r3, src, 3, m); \
    } while (0)
/* the 16-byte records the assembly writes with sq / sw */
#define SQ0(a) do { SD((a), 0); SD((a) + 8, 0); } while (0)
#define LDQ(q, a) do { (q)[0] = U32(LW((a))); (q)[1] = U32(LW((a) + 4)); (q)[2] = U32(LW((a) + 8)); (q)[3] = U32(LW((a) + 12)); } while (0)
#define SQV_Q(a, q) do { SW((a), (q)[0]); SW((a) + 4, (q)[1]); SW((a) + 8, (q)[2]); SW((a) + 12, (q)[3]); } while (0)

/* a0 mobys (0x100 bytes each), a1 output list, a2 count, a3 nonzero to queue the fading mobys
 * (+0x34 & 0xC00) for a second pass. Returns the end of the output list plus 0x10. */
char *func_L00_002569F8(char *a0, char *a1, int a2, int a3)
{
    float vf[32][4];
    float acc[4];
    float Q;
    unsigned int q9[4], q10[4], q11[4], q12[4], q13[4], q14[4];
    long r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20;
    long r23, r24, r25, r30, r31;
    long r14h, r15h;   /* high halves of $14 and $15 */
    long end, defp;    /* $21 low (end of the moby array) / high (queue write pointer) */
    long nlights;      /* $22 high; the low half is the scratchpad base */
    long cur, lim;     /* $29 low (chain write pointer) / high (chain limit) */
    int i;

    for (i = 0; i < 32; i++) {
        vf[i][0] = 0.0f; vf[i][1] = 0.0f; vf[i][2] = 0.0f; vf[i][3] = 0.0f;
    }
    vf[0][3] = 1.0f;
    acc[0] = acc[1] = acc[2] = acc[3] = 0.0f;
    Q = 0.0f;
    q9[0] = q9[1] = q9[2] = q9[3] = 0;
    q10[0] = q10[1] = q10[2] = q10[3] = 0;
    q11[0] = q11[1] = q11[2] = q11[3] = 0;
    q12[0] = q12[1] = q12[2] = q12[3] = 0;
    q13[0] = q13[1] = q13[2] = q13[3] = 0;
    q14[0] = q14[1] = q14[2] = q14[3] = 0;
    r1 = r2 = r3 = r4 = r5 = r6 = r7 = r8 = r9 = r10 = r11 = r12 = r13 = r14 = r15 = r16 = r17 = 0;
    r18 = r19 = r20 = 0;
    r14h = 0;
    r15h = 0;

    /* ---- setup ---- */
    r30 = (long)U32(a0) - 0x100;
    end = S32(((long)a2 << 8) + (long)U32(a0));
    r25 = U32(a1);
    r31 = a3 != 0 ? 0xC00 : 0;

    /* D9: 0x50 quads of D_L00_0017FFC0 to the scratchpad at 0x2000 (lights) */
    qcopy_2569F8(SPB + 0x2000, 0x17FFC0, 0x50);
    /* zero 0x2600..0x2800 */
    for (r1 = 0; r1 < 0x200; r1 += 8) {
        SD(SPB + 0x2600 + r1, 0);
    }
    qcopy_2569F8(SPB + 0x3800, 0x173E00, 8);   /* D_L00_00173E00, 0x80 bytes */
    qcopy_2569F8(SPB + 0x3300, 0x1AEF00, 4);   /* D_L00_001AEF00, 0x40 bytes */
    cur = LW(0x161280);                        /* chain write pointer (gp-0x5A80) */
    lim = LW(0x161288);                        /* chain limit (gp-0x5A78) */
    r15 = 0;
    r15h = SPB + 0x3400;
    qcopy_2569F8(SPB + 0x3F40, 0x1608D0, 1);   /* gp-0x6430 */
    vf[11][0] = vf[0][0] + vf[0][3];
    vf[11][1] = vf[0][1] - vf[0][3];
    r23 = LW(0x1600B0);                        /* gp-0x6C50 */
    r20 = 1;
    vf[1][0] = bf_2569F8(0x44800000u);
    vf[1][1] = 0.0f;
    defp = SPB + 0x3000;
    LQV(24, 0x166D80 + 0x140);
    LQV(25, 0x166D80 + 0x00);
    LQV(26, 0x166D80 + 0x10);
    LQV(27, 0x166D80 + 0x20);
    LQV(28, 0x166D80 + 0x40);
    LQV(29, 0x166D80 + 0x50);
    LQV(30, 0x166D80 + 0x60);
    LQV(31, 0x166D80 + 0x70);
    VB(O_MUL, 24, 24, 1, 0, M_XYZ);
    r24 = SPB + 0x2C00;
    VB(O_MUL, 24, 0, 1, 0, M_W);
    LQV(19, 0x16CB40 + 0xA0);
    LQV(20, 0x16CB40 + 0xB0);
    LQV(21, 0x16CB40 + 0x1D0);
    LQV(22, 0x16CB40 + 0x1E0);
    LQV(23, 0x16CB40 + 0x1F0);
    qcopy_2569F8(SPB + 0x3F00, 0x1C3240, 4);   /* D_L00_001C3240 */
    qcopy_2569F8(SPB + 0x3E80, 0x160930, 6);   /* gp-0x63D0.. */
    qcopy_2569F8(SPB + 0x3EE0, 0x160990, 1);   /* gp-0x6370 */

    /* chain: a cnt tag with MSCAL, then a ref tag of 4 quads from D_L00_001C2800 */
    SQ0(cur);
    SW(cur, 0x10000000);
    SH(cur + 0xE, 0x1400);
    SH(cur + 0xC, 0);
    cur += 0x10;
    SQ0(cur);
    SW(cur + 4, 0x1C2800);
    SW(cur, 0x30000004);
    cur += 0x10;
    qcopy_2569F8(U32(cur), SPB + 0x3F40, 1);
    qcopy_2569F8(U32(cur + 0x70), SPB + 0x3F40, 1);
    SB(cur + 0, 6);
    SB(cur + 0xE, 6);
    SB(cur + 0x70, 6);
    SB(cur + 0x7E, 6);
    SH(cur + 0xC, 0x123);
    SH(cur + 0x7C, 0x287);
    cur += 0xE0;
    /* six quads of D_L00_0016CB40 twice */
    qcopy_2569F8(U32(cur - 0xD0), 0x16CB40, 6);
    qcopy_2569F8(U32(cur - 0x60), 0x16CB40, 6);

    /* the lights: keep those whose second quad has a nonzero xy, scaled by 1024 */
    vf[3][0] = bf_2569F8(0x44800000u);
    vf[3][1] = 0.0f;
    vf[4][0] = bf_2569F8(0x3A800000u);
    vf[4][1] = 0.0f;
    r1 = SPB + 0x2400;
    r2 = SPB + 0x2500;
    r3 = SPB + 0x2400;
    r4 = 0;
    for (;;) {
        if (r1 == r2) {
            break;
        }
        r5 = LD(r1 + 0x10);
        r1 += 0x20;
        if (r5 == 0) {
            continue;
        }
        LQV(1, r1 - 0x10);
        LQV(2, r1 - 0x20);
        Q = vdiv_2569F8(vf[4][0], vf[1][3]);        /* vdiv Q, vf4x, vf1w */
        VB(O_MUL, 1, 1, 3, 0, M_XYZW);
        SQV(r3 + 0x10, 1);
        vf[2][3] = vf[0][3] * Q;                    /* vmulq.w vf2, vf0, Q */
        SQV(r3, 2);
        r4 += 1;
        r3 += 0x20;
    }
    nlights = r4;

    /* ---- the moby loop ---- */
L256D50:
    r2 = LH(r30 + 0x132);
    r30 += 0x100;
    if (r30 == end) {
        goto L257718;
    }
    LQV(1, r30);
    r1 = LB(r30 + 0x20);
    {
        int m = (int)r2 < (int)r23 ? (int)r2 : (int)r23;      /* pminw, word 0 */
        r2 = m;
        vf[4][0] = 0.0f;
        vf[4][1] = bf_2569F8((((unsigned int)m) & 0x3FFFFFu) << 10);   /* dsll32 by 10, qmtc2 */
        vf[4][2] = 0.0f;
        vf[4][3] = 0.0f;
    }
    VV(O_SUB, 2, 1, 24, M_XYZ);
    if (r1 < 0) {
        goto L257708;
    }
    VB(O_MUL, 3, 11, 1, 3, M_XY);
    vitof_2569F8(vf, 19, 4, 1.0f, M_Y);
    r6 = LBU(r30 + 0x37);
    VB(O_MULA, 0, 25, 2, 0, M_XYZ);
    r7 = LBU(r30 + 0x36);
    VB(O_MADDA, 0, 26, 2, 1, M_XYZ);
    r6 += SPB;
    VB(O_MADD, 2, 27, 2, 2, M_XYZ);
    r6 = LBU(r6 + 0x3800);
    VB(O_SUB, 19, 19, 1, 3, M_Y);
    r8 = LBU(r30 + 0x31);
    VB(O_MUL, 8, 22, 1, 3, M_XY);
    r7 &= r6;
    r14 = LHU(r30 + 0x34);
    if (r7 == 0) {
        goto L2576F8;
    }
    VB(O_ADD, 1, 3, 2, 2, M_XY);
    r7 = LBU(r30 + 0x22);
    VB(O_MUL, 5, 20, 2, 2, M_XY);
    r2 = r14 & 0x81;
    for (i = 0; i < 2; i++) {            /* vabs.xy vf2, vf2 (delay slot) */
        vf[2][i] = vf[2][i] < 0.0f ? -vf[2][i] : vf[2][i];
    }
    if (r2 != 0) {
        goto L2576F8;
    }
    VB(O_MUL, 7, 8, 22, 2, M_XY);
    r9 = r7 << 1;
    VV(O_SUB, 3, 19, 1, M_XY);
    r7 <<= 2;
    VV(O_MUL, 9, 5, 21, M_XY);
    r7 += SPB;
    VV(O_SUB, 6, 2, 8, M_XY);
    VV(O_ADD, 8, 2, 7, M_XY);
    r5 = 0x40000000;
    VV(O_SUB, 4, 1, 23, M_XY);
    r5 += r5;
    VV(O_MAX, 1, 0, 2, M_Z);
    r19 = 0xA;
    r1 = qlo_2569F8(vf[3]);
    r6 = r14 & r31;
    VV(O_SUB, 7, 5, 6, M_XY);
    VV(O_SUB, 8, 9, 8, M_XY);
    r2 = (long)((unsigned long)r1 << 32);
    r4 = qlo_2569F8(vf[4]);
    if (r1 < 0) {
        goto L2576F8;
    }
    {   /* vmr32.xy vf1, vf1 (delay slot of the next branch) */
        float y = vf[1][1], z = vf[1][2];
        vf[1][0] = y;
        vf[1][1] = z;
    }
    if (r2 >= 0) {
        goto L2576F8;
    }
    r2 = qlo_2569F8(vf[7]);
    if (r6 != 0) {
        goto L2578C8;
    }
    r3 = qlo_2569F8(vf[8]);
    r4 ^= r5;
    if (r2 < 0) {
        goto L2576F8;
    }
    r2 = (long)((unsigned long)r2 << 32);
    vftoi0_2569F8(vf, 1, 1, M_XY);
    if (r2 < 0) {
        goto L2576F8;
    }
    r3 |= r4;
    vftoi0_2569F8(vf, 3, 3, M_Y);
    if (r3 < 0) {
        r3 = (long)((unsigned long)r3 << 32);
        goto L256E70;
    }
    r3 = (long)((unsigned long)r3 << 32);
    if (r3 < 0) {
        goto L256E70;
    }
    r19 = 0xE;
L256E70:
    r1 = 1;
    if (r8 == 0) {
        SB(r30 + 0x31, r1);
    }

    /* ---- draw one moby (also entered from the queued pass) ---- */
L256E80:
    r1 = LW(r7 + 0x3A00);
    r18 = 2;
    LQV(18, r30 + 0x10);
    r2 = 0x14000000;
    r4 = LW(r30 + 0x2C);
    r19 |= r2;
    r2 = qlo_2569F8(vf[1]);
    VB(O_MUL, 18, 18, 24, 3, M_XYZ);
    r24 ^= 0x400;
    vf[10][0] = bf_2569F8(U32(r4));
    vf[10][1] = bf_2569F8(r4 < 0 ? 0xFFFFFFFFu : 0u);
    {
        int xv = (int)fb_2569F8(vf[1][0]);
        int tv = (int)r1;
        SW(r7 + 0x3A00, xv < tv ? xv : tv);       /* per-class minimum of the lod value */
    }
    r10 = LD(r30 + 0x38);
    r1 = qlo_2569F8(vf[3]);
    VV(O_SUB, 18, 18, 24, M_XYZ);
    r3 = LBU(r30 + 0x23);
    r1 = (long)((unsigned long)r1 >> 39);
    if (r1 > 0x80) {
        r1 = 0x80;
    }
    r4 = 0x47800000;
    {
        unsigned int prod = U32(r1 * r3);
        unsigned int w;
        unsigned int q[4];

        SQ0(r24 + 0x40); SQ0(r24 + 0x50); SQ0(r24 + 0x60); SQ0(r24 + 0x70);
        SQ0(r24 + 0x80); SQ0(r24 + 0x90); SQ0(r24 + 0xA0);
        r14 &= 0x208;
        r1 = r1 < 0x80;
        r1 <<= 3;
        r14 |= r1;
        w = U32(r10 >> 32) | ((prod >> 7) << 24);
        for (i = 0; i < 4; i++) {
            q[i] = 0x47800000u | ((w >> (8 * i)) & 0xFF);
        }
        *(unsigned int *)U32(r24 + 0xB0) = q[0];
        *(unsigned int *)U32(r24 + 0xB4) = q[1];
        *(unsigned int *)U32(r24 + 0xB8) = q[2];
        *(unsigned int *)U32(r24 + 0xBC) = q[3];
    }
    r1 = lim;
    LQV(15, r30 + 0xC0);
    LQV(16, r30 + 0xD0);
    r1 = S32(r1 - cur);
    LQV(17, r30 + 0xE0);
    if (r1 < 0) {
        goto L257718;
    }
    /* vf12..14 = minus the transpose of the 3x3 of vf15..17 */
    vf[12][0] = vf[0][0] - vf[15][0];
    vf[13][1] = vf[0][1] - vf[16][1];
    vf[14][2] = vf[0][2] - vf[17][2];
    vf[13][0] = vf[0][0] - vf[15][1];
    vf[14][0] = vf[0][0] - vf[15][2];
    vf[12][1] = vf[0][1] - vf[16][0];
    vf[14][1] = vf[0][1] - vf[16][2];
    vf[12][2] = vf[0][2] - vf[17][0];
    vf[13][2] = vf[0][2] - vf[17][1];
    VB(O_MUL, 15, 15, 10, 0, M_XYZ);
    VB(O_MUL, 16, 16, 10, 0, M_XYZ);
    VB(O_MUL, 17, 17, 10, 0, M_XYZ);
    MAT4(15, 28, 29, 30, 31, 15, M_XYZW);
    MAT4(16, 28, 29, 30, 31, 16, M_XYZW);
    MAT4(17, 28, 29, 30, 31, 17, M_XYZW);
    VB(O_MULA, 0, 28, 18, 0, M_XYZW);
    VB(O_MADDA, 0, 29, 18, 1, M_XYZW);
    VB(O_MADDA, 0, 30, 18, 2, M_XYZW);
    VB(O_MADD, 18, 31, 0, 3, M_XYZW);
    r17 = LW(r30 + 0x24);
    r15 = LBU(r30 + 0x73);
    r9 += SPB;
    r1 = LBU(r30 + 0x72);
    r9 += 0x2600;
    r8 = qlo_2569F8(vf[1]);
    if (r15 != 0) {
        goto L257B58;
    }
L257008:
    r3 = LW(r30 + 0x54);
    r4 = 0x30000;
    {   /* pminw / pmaxw on word 0, then sra */
        int w0 = (int)r8;
        if (w0 > 0x30000) {
            w0 = 0x30000;
        }
        if (w0 < 0) {
            w0 = 0;
        }
        r8 = w0;
    }
    r4 = LW(r30 + 0x68);
    r1 <<= 10;
    r5 = LW(r30 + 0x6C);
    r6 = LW(r30 + 0x60);
    r7 = LW(r30 + 0x64);
    r1 = r8 - r1;
    SW(r24 + 4, r3);
    SW(r24 + 8, r6);
    SW(r24 + 0xC, r7);
    r11 = LWU(r30 + 0x50);
    if (r1 > 0) {
        goto L257070;
    }
    r8 = LBU(r9 + 0);
    r14h = 0;
    r1 = 0;
    r2 = LBU(r17 + 4);
    r8 += 1;
    r6 = LBU(r17 + 8);
    SB(r9 + 0, r8);
    goto L257090;
L257070:
    r8 = LBU(r9 + 1);
    r7 = 1;
    r1 = LBU(r17 + 4);
    r2 = LBU(r17 + 5);
    r8 += 1;
    r6 = LBU(r17 + 9);
    SB(r9 + 1, r8);
    r14h = r7;
L257090:
    r12 = (long)((unsigned long)r11 >> 16);
    r8 = LW(r17 + 0x18);
    r12 &= 0xFF;
    r9 = LW(r17 + 0x14);
    r13 = (long)((unsigned long)r11 >> 24);
    r17 = LW(r17 + 0);
    SW(r24 + 0, r2);
    r1 <<= 4;
    SW(r24 + 0x10, r6);
    r2 <<= 4;
    SW(r24 + 0x14, r15);
    r17 += r1;
    SW(r24 + 0x18, r8);
    r16 = r17 + r2;
    SW(r24 + 0x1C, r9);
    {
        long z6 = r6;
        r6 = r11 & 0xFF;
        r7 = (long)((unsigned long)r11 >> 8);
        if (z6 == 0) {
            goto L257138;
        }
    }
    if (r12 != r13) {
        goto L2570F8;
    }
    r7 &= 0xFF;
    r6 += 1;
    {
        long cmp = (r6 != r7);
        r6 = 0x400;
        if (cmp) {
            goto L2570F8;
        }
    }
    SH(r24 + 0x12, r6);
L2570F8:
    r6 = LHU(r4 + 6);
    r7 = LW(r4 + 8);
    r8 = LW(r4 + 0xC);
    r4 += 0x10;
    SW(r24 + 0x20, r4);
    SW(r24 + 0x24, r6);
    SW(r24 + 0x28, r7);
    SW(r24 + 0x2C, r8);
    r6 = LHU(r5 + 6);
    r7 = LW(r5 + 8);
    r8 = LW(r5 + 0xC);
    r5 += 0x10;
    SW(r24 + 0x30, r5);
    SW(r24 + 0x34, r6);
    SW(r24 + 0x38, r7);
    SW(r24 + 0x3C, r8);
L257138:
    r13 = LHU(r30 + 0x34);
    r3 = (long)((unsigned long)(r10 & 0xFFFFFFFF) >> 16);
    r1 = r10 & 0xFF;
    r3 &= 0xFF;
    r1 <<= 6;
    r13 &= 0x10;
    r1 += SPB;
    if (r3 != 0) {
        goto L2571F0;
    }
    /* one joint matrix, no blend */
    LQV(1, r1 + 0x2010);
    LQV(2, r1 + 0x2030);
    MAT3(1, 12, 13, 14, 1, M_XYZW);
    LQV(3, r1 + 0x2000);
    LQV(4, r1 + 0x2020);
    SQV(r24 + 0x70, 3);
    SQV(r24 + 0x80, 4);
    MAT3(2, 12, 13, 14, 2, M_XYZW);
    r4 = LW(r1 + 0x200C);
    r5 = LW(r1 + 0x202C);
    SW(r24 + 0xA0, r4);
    SW(r24 + 0xA4, r5);
    SW(r24 + 0x7C, 0);
    SW(r24 + 0x8C, 0);
    SW(r24 + 0x40, fb_2569F8(vf[1][0]));
    SW(r24 + 0x50, fb_2569F8(vf[1][1]));
    SW(r24 + 0x60, fb_2569F8(vf[1][2]));
    SW(r24 + 0x44, fb_2569F8(vf[2][0]));
    SW(r24 + 0x54, fb_2569F8(vf[2][1]));
    SW(r24 + 0x64, fb_2569F8(vf[2][2]));
    goto L257330;
L2571F0:
    /* two joint matrices blended by r3/0x1000 */
    r3 <<= 4;
    r2 = r10 & 0xFF00;
    r4 = 0x1000;
    r2 = (long)((unsigned long)r2 >> 2);
    r4 -= r3;
    r2 += SPB;
    vf[10][0] = vf[0][0] + vf[0][3];
    vf[10][1] = vf[0][1] + vf[0][3];
    vf[10][2] = vf[0][2] + vf[0][3];
    vf[9][0] = bf_2569F8(U32(r3));
    vf[9][1] = 0.0f;
    vf[9][2] = bf_2569F8(U32(r4));
    vf[9][3] = 0.0f;
    LQV(1, r1 + 0x2010);
    LQV(2, r1 + 0x2030);
    LQV(3, r1 + 0x2000);
    LQV(4, r1 + 0x2020);
    vitof_2569F8(vf, 9, 9, 4096.0f, M_XZ);
    LQV(5, r2 + 0x2010);
    LQV(6, r2 + 0x2030);
    LQV(7, r2 + 0x2000);
    LQV(8, r2 + 0x2020);
    VB(O_MUL, 1, 1, 9, 2, M_XYZ);
    VB(O_MUL, 2, 2, 9, 2, M_XYZ);
    VB(O_MUL, 3, 3, 9, 2, M_XYZW);
    VB(O_MUL, 4, 4, 9, 2, M_XYZW);
    VB(O_MUL, 5, 5, 9, 0, M_XYZ);
    VB(O_MUL, 6, 6, 9, 0, M_XYZ);
    VB(O_MUL, 7, 7, 9, 0, M_XYZW);
    VB(O_MUL, 8, 8, 9, 0, M_XYZW);
    VV(O_ADD, 1, 1, 5, M_XYZ);
    VV(O_ADD, 2, 2, 6, M_XYZ);
    VV(O_ADD, 3, 3, 7, M_XYZW);
    VV(O_ADD, 4, 4, 8, M_XYZW);
    VV(O_MUL, 5, 1, 1, M_XYZ);
    VV(O_MUL, 6, 2, 2, M_XYZ);
    SQV(r24 + 0x70, 3);
    SQV(r24 + 0x80, 4);
    VB(O_ADDA, 0, 5, 5, 1, M_X);                   /* vadday.x ACC, vf5, vf5y */
    VB(O_MADD, 5, 10, 5, 2, M_X);                  /* vmaddz.x vf5, vf10, vf5z */
    VB(O_ADDA, 0, 6, 6, 1, M_X);
    VB(O_MADD, 6, 10, 6, 2, M_X);
    Q = vrsqrt_2569F8(vf[0][3], vf[5][0]);
    MAT3(1, 12, 13, 14, 1, M_XYZW);
    r1 = LW(r24 + 0x7C);
    r2 = LW(r24 + 0x8C);
    SW(r24 + 0x7C, 0);
    SW(r24 + 0x8C, 0);
    SW(r24 + 0xA0, r1);
    SW(r24 + 0xA4, r2);
    VQ(O_MUL, 1, 1, M_XYZ);
    Q = vrsqrt_2569F8(vf[0][3], vf[6][0]);
    MAT3(2, 12, 13, 14, 2, M_XYZW);
    SW(r24 + 0x40, fb_2569F8(vf[1][0]));
    SW(r24 + 0x50, fb_2569F8(vf[1][1]));
    SW(r24 + 0x60, fb_2569F8(vf[1][2]));
    VQ(O_MUL, 2, 2, M_XYZ);
    SW(r24 + 0x44, fb_2569F8(vf[2][0]));
    SW(r24 + 0x54, fb_2569F8(vf[2][1]));
    SW(r24 + 0x64, fb_2569F8(vf[2][2]));
L257330:
    LQV(1, r30);
    r3 = nlights;
    r24 += 0xC0;
    if (r3 == 0) {
        goto L2574F8;
    }
    LQV(2, SPB + 0x2410);
    r2 = SPB + 0x2400;
    r1 = SPB + 0x2500;
    r4 = 0;
    VV(O_SUB, 3, 2, 1, M_XYZ);
    vf[6][0] = vf[0][0] + vf[0][3];
    vf[6][1] = vf[0][1] + vf[0][3];
    vf[6][2] = vf[0][2] + vf[0][3];
    vf[7][0] = vf[0][0] - vf[0][3];
    vf[7][1] = vf[0][1] - vf[0][3];
    vf[7][2] = vf[0][2] - vf[0][3];
L257368:
    VV(O_MUL, 3, 3, 3, M_XYZ);                       /* delay slot of the exit test */
    if (r3 == 0) {
        goto L257440;
    }
    VV(O_MUL, 4, 2, 2, M_W);
    r2 += 0x20;
    LQV(2, r2 + 0x10);
    r3 -= 1;
    VB(O_ADDA, 0, 3, 3, 0, M_Y);                     /* vaddax.y ACC, vf3, vf3x */
    VB(O_MADDA, 0, 6, 3, 2, M_Y);                    /* vmaddaz.y ACC, vf6, vf3z */
    VB(O_MADD, 5, 7, 4, 3, M_Y);                     /* vmaddw.y vf5, vf7, vf4w */
    VB(O_MADD, 8, 0, 0, 0, M_Y);                     /* vmaddx.y vf8, vf0, vf0x */
    VV(O_SUB, 3, 2, 1, M_XYZ);
    r5 = qlo_2569F8(vf[5]);
    Q = vsqrt_2569F8(vf[8][1]);                      /* vsqrt Q, vf8y */
    if (r5 > 0) {
        goto L257368;
    }
    LQV(8, r2 - 0x10);
    LQV(9, r2 - 0x20);
    VV(O_SUB, 8, 1, 8, M_XYZ);
    vf[5][3] = vf[0][3] * Q;                         /* vmulq.w vf5, vf0, Q */
    vf[9][3] = vf[9][3] * Q;                         /* vmulq.w vf9, vf9, Q */
    Q = vdiv_2569F8(vf[0][3], vf[5][3]);             /* vdiv Q, vf0w, vf5w */
    vf[9][3] = vf[0][3] - vf[9][3];                  /* vsub.w vf9, vf0, vf9 */
    MAT3(8, 12, 13, 14, 8, M_XYZW);
    VB(O_MUL, 9, 9, 9, 3, M_XYZ);                    /* vmulw.xyz vf9, vf9, vf9w */
    VQ(O_MUL, 8, 8, M_XYZ);
    SQV(r1, 9);
    r4 += 1;
    SQV(r1 + 0x10, 8);
    r1 += 0x20;
    goto L257368;
L257440:
    r1 = 1;
    if (r4 == 0) {
        goto L2574F8;
    }
    r1 = SPB + 0x2500;
    if (r4 == 1) {
        qcopy_2569F8(U32(r24 - 0x30), U32(r1), 1);
        SW(r24 - 0x24, 0);
        SW(r24 - 0x78, LW(r1 + 0x10));
        SW(r24 - 0x68, LW(r1 + 0x14));
        SW(r24 - 0x58, LW(r1 + 0x18));
        goto L2574F8;
    }
    vf[1][0] = vf[0][0] + vf[0][0];
    vf[1][1] = vf[0][1] + vf[0][1];
    vf[1][2] = vf[0][2] + vf[0][2];
    VV(O_MULA, 0, 0, 0, M_XYZ);
    do {
        LQV(3, r1);
        LQV(4, r1 + 0x10);
        VV(O_ADD, 1, 1, 3, M_XYZ);
        r1 += 0x20;
        VB(O_MADDA, 0, 4, 3, 3, M_XYZ);              /* vmaddaw.xyz ACC, vf4, vf3w */
        r4 -= 1;
        VB(O_MADD, 2, 0, 0, 0, M_XYZ);               /* vmaddx.xyz vf2, vf0, vf0x (delay slot) */
    } while (r4 != 0);
    VV(O_MUL, 3, 2, 2, M_XYZ);
    VB(O_ADDA, 0, 3, 3, 1, M_X);                     /* vadday.x ACC, vf3, vf3y */
    VB(O_MADD, 3, 6, 3, 2, M_X);                     /* vmaddz.x vf3, vf6, vf3z */
    Q = vrsqrt_2569F8(vf[0][3], vf[3][0]);
    VQ(O_MUL, 2, 2, M_XYZ);
    SQV(r24 - 0x30, 1);
    SW(r24 - 0x24, 0);
    SW(r24 - 0x78, fb_2569F8(vf[2][0]));
    SW(r24 - 0x68, fb_2569F8(vf[2][1]));
    SW(r24 - 0x58, fb_2569F8(vf[2][2]));
L2574F8:
    LDQ(q9, SPB + 0x3F40);
    if (r13 != 0) {
        goto L257AE8;
    }
L257500:
    LDQ(q10, SPB + 0x3E80);
    LDQ(q11, SPB + 0x3E90);
    LDQ(q12, SPB + 0x3EC0);
    LDQ(q13, SPB + 0x3ED0);
L257510:
    r20 = S32(0 - r20);
    if (r18 > 0) {
        SQV_Q(cur, q9);
        r18 -= 1;
        SQV(cur + 0x10, 15);
        SQV(cur + 0x20, 16);
        SQV(cur + 0x30, 17);
        SQV(cur + 0x40, 18);
        r1 = 0x28D;
        if (!(r20 > 0)) {
            r1 = 0x129;
        }
        SH(cur + 0xC, r1);
        cur += 0x50;
    }
    r1 = LW(r17 + 8);
    r2 = LW(r17 + 0xC);
    r6 = cur + 0x20;
    SQV_Q(cur, q10);
    r1 <<= 4;
    SQV_Q(cur + 0x10, q11);
    r3 = r2 & 0xFF;
    r1 |= r3;
    r3 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 24);
    SW(r24 + 0, r1);
    r4 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 8);
    SW(r24 + 4, r6);
    r4 &= 0xFF;
    SB(cur + 0xE, r3);
    r5 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 16);
    SB(cur + 0x1E, r3);
    r5 &= 0xFF;
    SH(cur + 0, r4);
    r4 <<= 4;
    SH(cur + 0x10, r5);
    r5 <<= 4;
    r24 += 8;
    r6 += r5;
    SW(cur + 4, r6);
    r6 += r4;
    SW(cur + 0x14, r6);
    cur = r6;
    r2 = LD(r17 + 0);
    r17 += 0x10;
    SQV_Q(cur, q12);
    cur += 0x10;
    SW(cur - 0xC, r2);
    r3 = (long)((unsigned long)r2 >> 32);
    SH(cur - 0x10, r3);
    if (r18 > 0 && r14 != 0) {
        r1 = r14 & 0x200;
        SQ0(cur);
        SW(cur, 0x10000000);
        SW(cur + 0xC, 0x13000000);
        cur += 0x10;
        qcopy_2569F8(U32(cur), SPB + 0x3F00, 2);
        r4 = 0x48;
        if (r1 != 0) {
            /* low word 0x48, the other three words as stored at 0x3F20 */
            SW(cur + 0x20, 0x48);
            SW(cur + 0x24, 0);
            SW(cur + 0x28, LW(SPB + 0x3F28));
            SW(cur + 0x2C, LW(SPB + 0x3F2C));
        } else {
            long sum;
            r3 = r14 << 4;
            r3 -= 0x600;
            sum = S32(LW(SPB + 0x3F30) + r3);
            SW(cur + 0x20, sum);
            SW(cur + 0x24, sum < 0 ? -1 : 0);
            SW(cur + 0x28, LW(SPB + 0x3F38));
            SW(cur + 0x2C, LW(SPB + 0x3F3C));
        }
        cur += 0x30;
    }
    SQV_Q(cur, q13);
    cur += 0x10;
    SH(cur - 4, r19);
    if (r17 != r16) {
        goto L257510;
    }
    if (r14 != 0) {
        r1 = r14 & 0x200;
        SQ0(cur);
        SW(cur, 0x10000000);
        SW(cur + 0xC, 0x13000000);
        cur += 0x10;
        qcopy_2569F8(U32(cur), SPB + 0x3F00, 2);
        qcopy_2569F8(U32(cur + 0x20), SPB + (r1 != 0 ? 0x3F20 : 0x3F30), 1);
        cur += 0x30;
    }
    SD(r24 + 0, 0);                                  /* delay slot of the test below */
    if (r15 != 0) {
        goto L257B88;
    }
    r24 += 0x10;
L2576A0:
    {   /* D8: flush this moby's block of the scratchpad buffer to the output list */
        long size = r24 & 0x3FF;
        long base = r24 & 0x2C00;
        SD(r24 - 8, 0);
        qcopy_2569F8(U32(r25), SPB + U32(base), (int)(size >> 4));
        r25 += (size >> 4) << 4;
        r24 = base + SPB;
    }
    if (r31 >= 0) {
        goto L256D50;
    }
    goto L257878;
L2576F8:
    if (r8 == 0) {
        goto L256D50;
    }
    SB(r30 + 0x31, 0);
    goto L256D50;
L257708:
    r1 += 1;
    if (r1 != 0) {
        goto L256D50;
    }
L257718:
    if (r31 > 0) {
        goto L257848;
    }
L257720:
    r2 = r15h;
    r1 = SPB + 0x3400;
    r3 = S32(r2 - r1);
    SW(0x1600B8, r3);                                /* gp-0x6C48 */
    if (r3 > 0) {
        r3 = 0x1AC580;                               /* D_L00_001AC580 */
        do {
            qcopy_2569F8(U32(r3), U32(r1), 1);
            r1 += 0x10;
            r3 += 0x10;
        } while (r1 != r2);
    }
    r2 = defp;
    r4 = 0x1AC980;                                   /* D_L00_001AC980 */
    r1 = SPB + 0x3000;
    r3 = 0;
    if (r1 != r2) {
        do {
            r5 = LD(r1);
            r1 += 0x10;
            r6 = (long)((unsigned long)r5 >> 32);
            SD(r4, r5);
            if (r6 != 0) {
                r4 += 8;
                r3 += 1;
            }
        } while (r1 != r2);
    }
    SW(0x1600BC, r3);                                /* gp-0x6C44 */
    SQ0(cur);
    SW(cur, 0x10000000);
    SW(cur + 0xC, 0x13000000);
    cur += 0x10;
    SW(0x161280, cur);                               /* new chain pointer */
    r3 = r25;
    SQ0(SPB);
    SQ0(r3);                                         /* D8: one zero quad terminates the output list */
    return (char *)U32(r3 + 0x10);

    /* ---- the following catalogue entry (func_L00_00257848), inline ---- */
L257848:
    SQ0(cur);
    SW(cur, 0x10000000);
    SW(cur + 0xC, 0x13000000);
    cur += 0x10;
    SW(0x1600C0, cur);                               /* gp-0x6C40: start of the queued pass's chain */
    cur += 0x10;
    r31 = -1;
    r13 = SPB + 0x3000;
    SW(SPB + 0x3EF0, r13);
L257878:
    r13 = LW(SPB + 0x3EF0);
    r1 = defp;
    r30 = LW(r13 + 0);
    if (r13 == r1) {
        goto L257720;
    }
    r9 = LH(r13 + 8);
    r11 = LW(r13 + 0xC);
    r14 = LHU(r30 + 0x34);
    r7 = r9 << 1;
    r1 = LH(r30 + 0x32);
    r7 += SPB;
    r19 = LH(r13 + 0xA);
    r13 += 0x10;
    SW(SPB + 0x3EF0, r13);
    r1 <<= 10;
    r1 -= r11;
    if (r19 == 0) {
        goto L257878;
    }
    r1 = (long)((unsigned long)r1 << 32);
    vf[1][0] = bf_2569F8(U32(r11));
    vf[1][1] = bf_2569F8(r11 < 0 ? 0xFFFFFFFFu : 0u);
    vf[3][0] = 0.0f;
    vf[3][1] = bf_2569F8(U32(r1 >> 32));
    goto L256E80;

L2578C8:
    r19 = 0;
    vftoi0_2569F8(vf, 3, 3, M_Y);
    r3 = qlo_2569F8(vf[8]);
    r4 ^= r5;
    {
        long t2 = r2;
        r2 = (long)((unsigned long)r2 << 32);
        if (t2 < 0) {
            goto L257918;
        }
    }
    vftoi0_2569F8(vf, 1, 1, M_XY);
    if (r2 < 0) {
        goto L257918;
    }
    r19 = 0xA;
    r3 |= r4;
    if (r8 == 0) {
        r1 = 1;
        SB(r30 + 0x31, r1);
    }
    {
        long t3 = r3;
        r3 = (long)((unsigned long)r3 << 32);
        if (t3 < 0) {
            goto L257928;
        }
    }
    if (r3 < 0) {
        goto L257928;
    }
    r19 = 0xE;
    goto L257928;
L257918:
    if (r8 == 0) {
        goto L257928;
    }
    SB(r30 + 0x31, 0);
L257928:
    r13 = defp;
    r11 = qlo_2569F8(vf[1]);
    r2 = SPB + 0x3300;
    {
        int full = r13 == r2;
        r2 = LHU(r30 + 0x34);
        if (full) {
            goto L2576F8;
        }
    }
    SW(r13 + 0, r30);
    SH(r13 + 8, r9);
    SH(r13 + 0xA, r19);
    r2 &= 0x800;
    SW(r13 + 0xC, r11);
    if (r2 != 0) {
        goto L257AB0;
    }
    r4 = LW(r30 + 0x88);
    r3 = LW(r30 + 0x84);
    r2 = LBU(r30 + 0xBD);
    if (r4 <= 0) {
        goto L257AC8;
    }
    vf[4][0] = bf_2569F8(U32(r3));
    vf[4][1] = bf_2569F8(U32(r4));
    vf[4][2] = bf_2569F8(r3 < 0 ? 0xFFFFFFFFu : 0u);
    vf[4][3] = bf_2569F8(r4 < 0 ? 0xFFFFFFFFu : 0u);
    r2 <<= 4;
    r1 = LBU(r30 + 0x7F);
    r2 += SPB;
    VB(O_MUL, 4, 4, 24, 3, M_XY);
    vf[8][0] = bf_2569F8(0x3F000000u);
    vf[8][1] = 0.0f;
    LQV(5, r2 + 0x3300);
    LQV(6, r30);
    VB(O_SUB, 7, 4, 4, 0, M_Y);
    VB(O_MUL, 7, 7, 8, 0, M_Y);
    VB(O_ADD, 1, 6, 7, 1, M_W);
    VB(O_ADD, 4, 4, 7, 1, M_X);
    VB(O_SUB, 4, 4, 6, 2, M_X);
    Q = vdiv_2569F8(vf[4][0], vf[5][2]);
    VQ(O_MULA, 0, 5, M_XYZ);
    VB(O_MADD, 1, 6, 0, 3, M_XYZ);
    VV(O_SUB, 2, 1, 24, M_XYZ);
    vf[4][0] = 0.0f;
    vf[4][1] = bf_2569F8(U32(r1 << 10));
    vf[4][2] = 0.0f;
    vf[4][3] = 0.0f;
    VB(O_MUL, 3, 11, 1, 3, M_XY);
    vitof_2569F8(vf, 19, 4, 1.0f, M_Y);
    MAT3(2, 25, 26, 27, 2, M_XYZ);
    VB(O_SUB, 19, 19, 1, 3, M_Y);
    VB(O_MUL, 8, 22, 1, 3, M_XY);
    VB(O_ADD, 1, 3, 2, 2, M_XY);
    VB(O_MUL, 5, 20, 2, 2, M_XY);
    for (i = 0; i < 2; i++) {
        vf[2][i] = vf[2][i] < 0.0f ? -vf[2][i] : vf[2][i];
    }
    VB(O_MUL, 7, 8, 22, 2, M_XY);
    VV(O_SUB, 3, 19, 1, M_XY);
    VV(O_MUL, 9, 5, 21, M_XY);
    VV(O_SUB, 6, 2, 8, M_XY);
    VV(O_ADD, 8, 2, 7, M_XY);
    r5 = 0x40000000;
    VV(O_SUB, 4, 1, 23, M_XY);
    r5 += r5;
    r1 = qlo_2569F8(vf[3]);
    vf[1][0] = vf[1][1];                             /* vmr32.x vf1, vf1 */
    VV(O_SUB, 7, 5, 6, M_XY);
    VV(O_SUB, 8, 9, 8, M_XY);
    r2 = (long)((unsigned long)r1 << 32);
    r10 = 1;
    r4 = qlo_2569F8(vf[4]);
    if (r1 < 0) {
        goto L257AC8;
    }
    {
        long c = r2;
        r2 = qlo_2569F8(vf[7]);
        if (c >= 0) {
            goto L257AC8;
        }
    }
    r3 = qlo_2569F8(vf[8]);
    r4 ^= r5;
    if (r2 < 0) {
        goto L257AC8;
    }
    r2 = (long)((unsigned long)r2 << 32);
    r3 |= r4;
    if (r2 < 0) {
        goto L257AC8;
    }
    vftoi0_2569F8(vf, 1, 1, M_X);
    {
        long t3 = r3;
        r3 = (long)((unsigned long)r3 << 32);
        if (t3 < 0) {
            goto L257A78;
        }
    }
    if (r3 < 0) {
        goto L257A78;
    }
    r10 = 0;
L257A78:
    r3 = LBU(r30 + 0x7F);
    r4 = S32(qlo_2569F8(vf[1]));
    r3 <<= 10;
    r3 = S32(r3 - r4);
    r3 = (long)((unsigned int)r3 >> 1);
    if (r3 > 0x1000) {
        r3 = 0x1000;
    }
    SH(r13 + 6, r10);
    SH(r13 + 4, r3);
    r13 += 0x10;
    defp = r13;
    goto L256D50;
L257AB0:
    SH(r13 + 6, 0);
    if (r19 == 0) {
        goto L256D50;
    }
    SH(r13 + 4, 0);
    r13 += 0x10;
    defp = r13;
    goto L256D50;
L257AC8:
    r1 = LH(r30 + 0x32);
    if (r19 == 0) {
        goto L256D50;
    }
    r1 <<= 10;
    r1 -= r11;
    r1 = (long)((unsigned long)r1 << 32);
    vf[1][0] = bf_2569F8(U32(r11));
    vf[1][1] = bf_2569F8(r11 < 0 ? 0xFFFFFFFFu : 0u);
    vf[3][0] = 0.0f;
    vf[3][1] = bf_2569F8(U32(r1 >> 32));
    goto L256E80;
L257AE8:
    r1 = r15h;
    r3 = LW(r30 + 0x24);
    r2 = SPB + 0x3800;
    r6 = LW(r30 + 0x90);
    r4 = r14h;
    if (r1 == r2) {
        goto L257500;
    }
    r7 = LBU(r24 - 4);
    r3 += r4;
    r3 = LBU(r3 + 0xA);
    r5 = r16 - r17;
    r4 = r3 << 3;
    r3 <<= 4;
    r5 -= r3;
    r3 = r17 + r3;
    {
        long c = r5;
        r5 = (long)((unsigned int)r5 >> 4);
        if (c <= 0) {
            goto L257500;
        }
    }
    SW(r1 + 0, r6);
    r4 = r25 + r4;
    SB(r1 + 3, r7);
    SW(r1 + 4, r3);
    r4 += 0xC0;
    SW(r1 + 8, r4);
    SW(r1 + 0xC, r5);
    r1 += 0x10;
    r15h = r1;
    goto L257500;
L257B58:
    r3 = S32(r15 << 10);
    r4 = S32(r8);
    r3 = S32(r3 - r4);
    {
        long z = r3;
        r15 = 0;
        if (z <= 0) {
            goto L257008;
        }
    }
    r3 = (long)((unsigned int)r3 >> 7);
    if (r3 > 0x80) {
        r3 = 0x80;
    }
    r15 = r3;
    goto L257008;
L257B88:
    LQV(9, r30);
    r4 = r24 & 0x3FF;
    r1 = LW(r30 + 0x24);
    r4 = r24 - r4;
    r2 = LBU(r1 + 7);
    r3 = LBU(r1 + 6);
    r24 += 0x10;
    r17 = LW(r1 + 0);
    r2 <<= 4;
    r5 = r3 + 0xA;
    r3 <<= 4;
    VV(O_SUB, 9, 9, 24, M_XYZ);
    r5 = (long)((unsigned long)r5 >> 1);
    r17 += r2;
    SH(r4 + 0x16, r5);
    r16 = r17 + r3;
    r24 = (long)((unsigned long)(r24 & 0xFFFFFFFF) >> 4);
    LDQ(q10, SPB + 0x3E80);
    r24 = r24 << 4;
    LDQ(q11, SPB + 0x3EA0);
    VV(O_MUL, 5, 9, 9, M_XYZ);
    LDQ(q12, SPB + 0x3EB0);
    vf[6][0] = vf[0][0] + vf[0][3];
    vf[6][1] = vf[0][1] + vf[0][3];
    vf[6][2] = vf[0][2] + vf[0][3];
    LDQ(q13, SPB + 0x3EC0);
    LDQ(q14, SPB + 0x3ED0);
    VB(O_ADDA, 0, 5, 5, 1, M_X);
    VB(O_MADD, 5, 6, 5, 2, M_X);
    LQV(1, r30 + 0xC0);
    LQV(2, r30 + 0xD0);
    LQV(3, r30 + 0xE0);
    Q = vrsqrt_2569F8(vf[0][3], vf[5][0]);
    MAT3(1, 25, 26, 27, 1, M_XYZW);
    MAT3(2, 25, 26, 27, 2, M_XYZW);
    MAT3(3, 25, 26, 27, 3, M_XYZW);
    vf[4][0] = vf[0][0]; vf[4][1] = vf[0][1]; vf[4][2] = vf[0][2]; vf[4][3] = vf[0][3];
    vf[5][0] = vf[0][0] + vf[0][0]; vf[5][1] = vf[0][1] + vf[0][1]; vf[5][2] = vf[0][2] + vf[0][2];
    vf[6][0] = vf[0][0] + vf[0][0]; vf[6][1] = vf[0][1] + vf[0][1]; vf[6][2] = vf[0][2] + vf[0][2];
    vf[7][0] = vf[0][0] + vf[0][0]; vf[7][1] = vf[0][1] + vf[0][1]; vf[7][2] = vf[0][2] + vf[0][2];
    VQ(O_MUL, 9, 9, M_XYZ);
    MAT3(9, 25, 26, 27, 9, M_XYZW);
    vf[5][0] = vf[0][0] + vf[0][3];
    vf[6][1] = vf[0][1] + vf[9][2];
    vf[7][1] = vf[0][1] - vf[9][1];
    vf[6][2] = vf[0][2] + vf[9][1];
    vf[7][2] = vf[0][2] + vf[9][2];
    MAT3(1, 5, 6, 7, 1, M_XYZW);
    MAT3(2, 5, 6, 7, 2, M_XYZW);
    MAT3(3, 5, 6, 7, 3, M_XYZW);
    vf[5][0] = vf[0][0] + vf[9][2];
    vf[6][0] = vf[0][0] + vf[0][0]; vf[6][1] = vf[0][1] + vf[0][1]; vf[6][2] = vf[0][2] + vf[0][2];
    vf[7][0] = vf[0][0] - vf[9][0];
    vf[5][2] = vf[0][2] + vf[9][0];
    vf[6][1] = vf[0][1] + vf[0][3];
    vf[7][1] = vf[0][1] + vf[0][1];
    vf[7][2] = vf[0][2] + vf[9][2];
    MAT3(1, 5, 6, 7, 1, M_XYZW);
    MAT3(2, 5, 6, 7, 2, M_XYZW);
    MAT3(3, 5, 6, 7, 3, M_XYZW);
    SQV(r24 + 0x00, 1);
    SQV(r24 + 0x10, 2);
    SQV(r24 + 0x20, 3);
    SQV(r24 + 0x30, 4);
    r24 += 0x40;
    LQV(1, SPB + 0x3EE0);
    LDQ(q9, SPB + 0x3F40);
    VV(O_ADD, 18, 18, 1, M_XYZW);
    r18 = 2;
    r19 = 0x1400000A;
L257D08:
    r20 = S32(0 - r20);
    if (r18 > 0) {
        SQV_Q(cur, q9);
        r18 -= 1;
        SQV(cur + 0x10, 15);
        SQV(cur + 0x20, 16);
        SQV(cur + 0x30, 17);
        SQV(cur + 0x40, 18);
        r1 = 0x28D;
        if (!(r20 > 0)) {
            r1 = 0x129;
        }
        SH(cur + 0xC, r1);
        cur += 0x50;
    }
    r1 = LW(r17 + 8);
    r2 = LW(r17 + 0xC);
    SQV_Q(cur, q10);
    r1 <<= 4;
    SQV_Q(cur + 0x10, q11);
    r3 = r2 & 0xFF;
    SQV_Q(cur + 0x20, q12);
    r6 = cur + 0x30;
    r1 |= r3;
    r3 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 24);
    SW(r24 + 0, r1);
    r4 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 8);
    SW(r24 + 4, r6);
    r4 &= 0xFF;
    SB(cur + 0xE, r3);
    r5 = (long)((unsigned long)(r2 & 0xFFFFFFFF) >> 16);
    SB(cur + 0x1E, r3);
    r5 &= 0xFF;
    SB(cur + 0x2E, r3);
    SH(cur + 0, r4);
    r4 <<= 4;
    SH(cur + 0x10, r5);
    SH(cur + 0x20, r5);
    r5 <<= 4;
    r24 += 8;
    r6 += r5;
    SW(cur + 4, r6);
    r6 += r4;
    SW(cur + 0x14, r6);
    r6 += r5;
    SW(cur + 0x24, r6);
    cur = r6;
    r2 = LD(r17 + 0);
    r17 += 0x10;
    SQV_Q(cur, q13);
    cur += 0x10;
    SW(cur - 0xC, r2);
    r3 = (long)((unsigned long)r2 >> 32);
    SH(cur - 0x10, r3);
    SQV_Q(cur, q14);
    cur += 0x10;
    SH(cur - 4, r19);
    if (r17 != r16) {
        goto L257D08;
    }
    SD(r24 + 0, 0);
    r24 += 0x10;
    goto L2576A0;
}
