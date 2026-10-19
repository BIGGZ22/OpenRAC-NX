/* NON_MATCHING func_L00_002D8898 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: BYTES 34/2052 (98.3% of the bytes match), checked 2026-10-14.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - prologue: m1[0] addr in $s1 / m2[0] in $s2, retail swapped. Cause: GCSE copies the prologue's sp+0x20 pseudo
 *   main-loop mat select (m2 : m1) and third-loop matrix arg, giving it 8 refs; local-alloc then ranks it above m2
 *   Removing every later m1 reference flips it (regalloc probes probe1..5, g1, g2, h1, y1, y2: no legal form found
 *   - third loop GIVs (c/u/v/idx in $s5/$s2/$s3/$s1 vs retail $s3/$s1/$s2/$s5): follows from the above (pseudo cop
 *   - D3010 branch: 0x8000000044 built in $v0 (retail $v1) and k=2 scheduled before addiu $fp; local-alloc tie (eq
 *   Note: file declares `void func_L00_002D8898(void);` and P/Q/T; candidate defines func_L00_002D8898_r(P *) __as
 *   and its own Q_2D8898 (q fields 0x54..0x5F not in file's Q).
 *   Mistake: one awk command wrote a scratch copy of a dump to /tmp/x (not deleted, per no-delete rule).
 *   fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18).
 */
typedef struct {
    s16 start; s16 count; int idx; char pad08[0x30];
    int col; /* 38 */
    char pad3C[4];
    s16 extra; s16 flag42; /* 40 42 */
    float f44, f48, f4C, f50; /* 44..50 */
    int tex0; /* 54 */
    int tex1; /* 58 */
    u8 c5C, c5D, c5E, c5F;
} Q_2D8898;
typedef int u128_2D8898 __attribute__((mode(TI)));
typedef float V_2D8898[4] __attribute__((aligned(16)));
typedef struct {
    V_2D8898 v[4];
    int c[4];
    float uv[4][2];
    long g[4];
} Quad_2D8898;

extern Vec4f D_L00_00166EC0_D83D8 __asm__("D_L00_00166EC0");
extern Vec4f D_L00_001CBAE0_D83D8[] __asm__("D_L00_001CBAE0");
extern u8 D_L00_001D2B60[];
extern s16 D_L00_001D5A40[];
extern float D_L00_001D05E0_2D90A0[] __asm__("D_L00_001D05E0");
extern float D_L00_001D18A0_2D90A0[] __asm__("D_L00_001D18A0");
extern float D_L00_001D34C0_2D90A0[] __asm__("D_L00_001D34C0");
extern float D_L00_001D4780_2D90A0[] __asm__("D_L00_001D4780");
extern s16 D_L00_001D63A0_2D90A0[] __asm__("D_L00_001D63A0");
extern u8 D_L00_001D3010_2D90A0[] __asm__("D_L00_001D3010");
extern s16 D_L00_001D6D00_2D90A0[] __asm__("D_L00_001D6D00");
extern float D_0015EE60 MACRO_ADDR;
extern float D_L00_0016143C MACRO_ADDR;
extern float D_L00_0016144C MACRO_ADDR;
extern float D_L00_00161454 MACRO_ADDR;
extern unsigned short D_L00_00161450;
extern short D_L00_00161A68;
extern short D_L00_00161A6C;
extern u8 D_L00_00161A70;
extern u8 D_L00_00161A78;
extern int D_L00_001E16D0[][4];
extern float D_L00_001E1610[][4];
extern float D_L00_001E1790[];
extern float D_L00_001E1794[];
extern float D_L00_001E14F0[][4];
extern float D_L00_001E1530[];
extern float D_L00_001E1550[][4];
extern float D_L00_001E1730[];
extern float D_L00_001E1734[];

extern void func_001F9BF0_2D3F40(void *, void *, void *) __asm__("func_001F9BF0");
extern void func_L00_001FF4B0_2D3F40(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9CA0_2D3F40(void *, void *, void *) __asm__("func_001F9CA0");
extern void func_001F9C30(void *, void *, float);
extern int func_001F4868_2D3F40(int) __asm__("func_001F4868");
extern void func_00234C98(int, long);
void func_001F9EE8(Vec4f *, Vec4f *, void *);
extern void func_L00_001FD1D8_2D3F40(void *, void *, int) __asm__("func_L00_001FD1D8");
extern int func_001F9938(void *);
extern float func_001FA888(int);
int func_002140B0(int);
extern int func_001FA898_2D3F40(float) __asm__("func_001FA898");

/* Draw callback of the waterfall foam emitter: camera-facing quads for each live particle, then the base mist ring. */
void func_L00_002D8898_r(P *p) __asm__("func_L00_002D8898");
void func_L00_002D8898_r(P *p) {
    V_2D8898 a, b;
    V_2D8898 m1[4], m2[4];
    Quad_2D8898 quad;
    Q_2D8898 *q = (Q_2D8898 *)p->q;
    T *t;
    int alpha, colA, end;
    V_2D8898 *mat;
    int alpha2, i, k, j, n, l;
    float sx, sy, f, one;
    float *src;

    alpha = q->c5C << 24;
    colA = q->c5F << 16 | q->c5E << 8 | q->c5D;
    t = &D_L00_001601AC[q->idx];
    end = q->start + q->count + q->extra;
    qcopy(a, &p->pos);
    *(u128_2D8898 *)b = 0;
    a[3] = 1.0f;
    b[2] = 1.0f;
    func_001F9BF0_2D3F40(m1[0], &D_L00_00166EC0_D83D8, a);
    func_L00_001FF4B0_2D3F40(m1[0], m1[0], 1.0f);
    func_001F9CA0_2D3F40(m1[1], m1[0], b);
    func_L00_001FF4B0_2D3F40(m1[1], m1[1], 1.0f);
    func_001F9CA0_2D3F40(m1[2], m1[1], m1[0]);
    qcopy(m1[3], a);
    func_001F9C30(m2[0], m1[0], -1.0f);
    func_001F9CA0_2D3F40(m2[1], m2[0], b);
    func_L00_001FF4B0_2D3F40(m2[1], m2[1], 1.0f);
    func_001F9CA0_2D3F40(m2[2], m2[1], m2[0]);
    qcopy(m2[3], m1[3]);
    quad.g[1] = func_001F4868_2D3F40(q->tex0 + 0x28);
    quad.g[2] = 0xFF9000000260L;
    quad.g[3] = 0x8000000048L;
    quad.g[0] = 0;
    func_00234C98(0x4A, 0);
    func_00234C98(0x47, 0x51001);
    for (i = q->start; i < end; i++) {
        f = D_L00_001D34C0_2D90A0[i] - D_L00_001D4780_2D90A0[i] * D_0015EE60;
        D_L00_001D34C0_2D90A0[i] = f;
        mat = D_L00_001D2B60[i] ? m2 : m1;
        if (f <= -8.0f) {
            D_L00_001D34C0_2D90A0[i] = f + 8.0f;
        }
        func_001F9EE8((Vec4f *)mat[3], &D_L00_001CBAE0_D83D8[i], t);
        sx = D_L00_001D05E0_2D90A0[i];
        sy = D_L00_001D18A0_2D90A0[i];
        if (D_L00_001D3010_2D90A0[i]) {
            quad.g[3] = 0x8000000044L;
            one = 1.0f;
            for (k = 2; k < 4; k++) {
                for (j = 0; j < 4; j++) {
                    n = D_L00_001E16D0[k][j];
                    src = D_L00_001E1610[n];
                    quad.v[j][0] = src[0];
                    quad.v[j][1] = src[1] * sx;
                    quad.v[j][2] = src[2] * sy;
                    quad.v[j][3] = one;
                    func_001F9EE8((Vec4f *)quad.v[j], (Vec4f *)quad.v[j], mat);
                    quad.uv[j][0] = D_L00_001E1794[n * 2] + D_L00_001D34C0_2D90A0[i];
                    quad.uv[j][1] = D_L00_001E1790[n * 2];
                    quad.c[j] = alpha * (&D_L00_00161A78)[n] | colA;
                }
                func_L00_001FD1D8_2D3F40(&quad, 0, 0);
            }
        } else {
            if (D_L00_001D5A40[i]) {
                func_001F9938(&D_L00_001D5A40[i]);
                alpha2 = func_001FA898_2D3F40((1.0f - func_001FA888(D_L00_001D5A40[i]) * D_L00_0016144C) * *(float *)&D_L00_00161A6C);
            } else if (D_L00_001D63A0_2D90A0[i]) {
                if (func_001F9938(&D_L00_001D63A0_2D90A0[i])) {
                    func_L00_002D90A0(p, i);
                    alpha2 = 0;
                    D_L00_001D2B60[i] = func_002140B0(2);
                } else {
                    alpha2 = func_001FA898_2D3F40(func_001FA888(D_L00_001D63A0_2D90A0[i]) * D_L00_00161454 * *(float *)&D_L00_00161A6C);
                }
            } else {
                if (func_001F9938(&D_L00_001D6D00_2D90A0[i])) {
                    D_L00_001D63A0_2D90A0[i] = D_L00_00161450;
                }
                alpha2 = *(int *)&D_L00_00161A68;
            }
            alpha2 <<= 24;
            quad.g[3] = 0x8000000048L;
            one = 1.0f;
            for (l = 0; l < 4; l++) {
                quad.v[l][0] = D_L00_001E14F0[l][0];
                quad.v[l][1] = D_L00_001E14F0[l][1] * sx;
                quad.v[l][2] = D_L00_001E14F0[l][2] * sy;
                quad.v[l][3] = one;
                quad.uv[l][0] = D_L00_001E1530[l * 2] - D_L00_001D34C0_2D90A0[i];
                quad.uv[l][1] = D_L00_001E1530[l * 2 + 1];
                quad.c[l] = alpha2 * (&D_L00_00161A70)[l] | q->col;
                func_001F9EE8((Vec4f *)quad.v[l], (Vec4f *)quad.v[l], mat);
            }
            func_L00_001FD1D8_2D3F40(&quad, 0, 0);
        }
    }
    quad.g[1] = func_001F4868_2D3F40(q->tex1 + 0x28);
    quad.g[3] = 0x8000000044L;
    qcopy(m1[3], t->pos);
    m1[3][2] -= q->f50;
    if (q->flag42 == 0) {
        for (k = 0; k < 6; k++) {
            for (j = 0; j < 4; j++) {
                n = D_L00_001E16D0[k][j];
                src = D_L00_001E1550[n];
                quad.v[j][0] = src[0];
                quad.v[j][1] = src[1] * q->f48;
                quad.v[j][2] = src[2] * q->f4C;
                quad.v[j][3] = 1.0f;
                func_001F9EE8((Vec4f *)quad.v[j], (Vec4f *)quad.v[j], m1);
                quad.uv[j][0] = D_L00_001E1734[n * 2] + D_L00_0016143C;
                quad.uv[j][1] = D_L00_001E1730[n * 2];
                quad.c[j] = (&D_L00_00161A78)[n] * 0xFF000000 | colA;
            }
            func_L00_001FD1D8_2D3F40(&quad, 0, 0);
        }
    }
}
