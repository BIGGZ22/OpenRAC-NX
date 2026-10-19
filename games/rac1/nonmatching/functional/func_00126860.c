/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten MMI (MPEG motion compensation, 16-wide horizontal half-pel): per row, X = 16 bytes at (shift & 15) of src1[0..15],src2[0..15]; Y = the same row shifted one more byte, whose last byte comes from src2[shift & 15] (retail's second funnel shift); output = (X + Y + 1) >> 1 per halfword, 32 bytes. a[2] rows, then (if nonzero) a[3] rows after skipping 0x80 source bytes. lq ignores the low 4 address bits (kept). MMI becomes plain halfword code; field meanings inferred from the registers. */
typedef struct {
    u16 *dst;   /* 0x00 */
    s32 shift;  /* 0x04 */
    s32 rows;   /* 0x08 */
    s32 rows2;  /* 0x0C */
    s32 stride; /* 0x10 */
    u8 *src1;   /* 0x14 */
    u8 *src2;   /* 0x18 */
} MC_126860;

static void row_126860(u16 *d, const u8 *s1, const u8 *s2, s32 n) {
    u8 c[32];
    u8 e[32];
    u8 x[16];
    u8 z[16];
    u8 w[16];
    s32 i;
    s1 = (const u8 *)((u32)s1 & ~15u);
    s2 = (const u8 *)((u32)s2 & ~15u);
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
        d[i] = (u16)(x[i] + w[i] + 1) >> 1;
    }
}

void func_00126860(MC_126860 *a) {
    u8 *s1 = a->src1;
    u8 *s2 = a->src2;
    u8 *d = (u8 *)a->dst;
    s32 n = a->shift & 15;
    s32 stride = a->stride;
    s32 cnt = a->rows;
    s32 first = 1;
    for (;;) {
        do {
            row_126860((u16 *)d, s1, s2, n);
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
