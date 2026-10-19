/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten MMI (MPEG motion compensation, 16-wide vertical half-pel): each row is 16 bytes taken from the 32 bytes src1[0..15],src2[0..15] starting at byte (shift & 15), widened to 16 halfwords; output row = (previous row + this row + 1) >> 1 per halfword, written as 32 bytes. a[2] rows, then (if nonzero) a[3] rows after skipping 0x80 source bytes, carrying the previous row over. lq ignores the low 4 address bits (kept). MMI becomes plain halfword code; field meanings inferred from the registers. */
typedef struct {
    u16 *dst;   /* 0x00 */
    s32 shift;  /* 0x04 */
    s32 rows;   /* 0x08 */
    s32 rows2;  /* 0x0C */
    s32 stride; /* 0x10 */
    u8 *src1;   /* 0x14 */
    u8 *src2;   /* 0x18 */
} MC_1266D8;

static void load_1266D8(u16 *out, const u8 *s1, const u8 *s2, s32 n) {
    u8 c[32];
    s32 i;
    s1 = (const u8 *)((u32)s1 & ~15u);
    s2 = (const u8 *)((u32)s2 & ~15u);
    for (i = 0; i < 16; i++) {
        c[i] = s1[i];
        c[16 + i] = s2[i];
    }
    for (i = 0; i < 16; i++) {
        out[i] = c[n + i];
    }
}

void func_001266D8(MC_1266D8 *a) {
    u8 *s1 = a->src1;
    u8 *s2 = a->src2;
    u8 *d = (u8 *)a->dst;
    s32 n = a->shift & 15;
    s32 stride = a->stride;
    s32 cnt = a->rows;
    s32 first = 1;
    u16 prev[16];
    u16 cur[16];
    s32 i;

    load_1266D8(prev, s1, s2, n);
    if (cnt == 0) {
        goto after;
    }
    for (;;) {
        do {
            s1 += stride;
            s2 += stride;
            load_1266D8(cur, s1, s2, n);
            cnt--;
            for (i = 0; i < 16; i++) {
                ((u16 *)d)[i] = (u16)(prev[i] + cur[i] + 1) >> 1;
                prev[i] = cur[i];
            }
            d += stride * 2;
        } while (cnt > 0);
    after:
        s1 += 0x80;
        s2 += 0x80;
        cnt = a->rows2;
        if (!first || cnt == 0) {
            break;
        }
        first = 0;
    }
}
