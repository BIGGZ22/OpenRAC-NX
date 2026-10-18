/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126860.c (games/rac1): func_00126860, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI (MPEG motion compensation, 16-wide horizontal half-pel): per row, X = 16 bytes at (shift & 15) of src1[0..15],src2[0..15]; Y = the same row shifted one more byte, whose last byte comes from src2[shift & 15] (retail's second funnel shift); output = (X + Y + 1) >> 1 per halfword, 32 bytes. a[2] rows, then (if nonzero) a[3] rows after skipping 0x80 source bytes. lq ignores the low 4 address bits (kept). MMI becomes plain halfword code; field meanings inferred from the registers. */

typedef int s32_c126720;
typedef unsigned char u8_c126720;
typedef unsigned short u16_c126720;
typedef unsigned int u32_c126720;
typedef struct {
    u16_c126720 *dst;
    s32_c126720 shift;
    s32_c126720 rows;
    s32_c126720 rows2;
    s32_c126720 stride;
    u8_c126720 *src1;
    u8_c126720 *src2;
} MC_126860_c126720;
static void row_126860_c126720(u16_c126720 *d, const u8_c126720 *s1, const u8_c126720 *s2, s32_c126720 n) {
    u8_c126720 c[32];
    u8_c126720 e[32];
    u8_c126720 x[16];
    u8_c126720 z[16];
    u8_c126720 w[16];
    s32_c126720 i;
    s1 = (const u8_c126720 *)((u32_c126720)s1 & ~15u);
    s2 = (const u8_c126720 *)((u32_c126720)s2 & ~15u);
    for (i = 0; i < 16; i++) {
        c[i] = s1[i];
        c[16 + i] = s2[i];
        e[i] = s2[i];
        e[16 + i] = s1[i];
    }
    for (i = 0; i < 16; i++) {
        x[i] = c[n + i];
        z[i] = e[n + i];
    }
    for (i = 0; i < 16; i++) {
        w[i] = (i + 1 < 16) ? x[i + 1] : z[0];
    }
    for (i = 0; i < 16; i++) {
        d[i] = (u16_c126720)(x[i] + w[i] + 1) >> 1;
    }
}
void func_00126720(MC_126860_c126720 *a) {
    u8_c126720 *s1 = a->src1;
    u8_c126720 *s2 = a->src2;
    u8_c126720 *d = (u8_c126720 *)a->dst;
    s32_c126720 n = a->shift & 15;
    s32_c126720 stride = a->stride;
    s32_c126720 cnt = a->rows;
    s32_c126720 first = 1;
    for (;;) {
        do {
            row_126860_c126720((u16_c126720 *)d, s1, s2, n);
            cnt--;
            s1 += stride;
            s2 += stride;
            d += stride * 2;
        } while (cnt > 0);
        s1 += 0x80;
        s2 += 0x80;
        cnt = a->rows2;
        if (!first || cnt == 0) {
            break;
        }
        first = 0;
    }
}
