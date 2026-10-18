/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001266D8.c (games/rac1): func_001266D8, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI (MPEG motion compensation, 16-wide vertical half-pel): each row is 16 bytes taken from the 32 bytes src1[0..15],src2[0..15] starting at byte (shift & 15), widened to 16 halfwords; output row = (previous row + this row + 1) >> 1 per halfword, written as 32 bytes. a[2] rows, then (if nonzero) a[3] rows after skipping 0x80 source bytes, carrying the previous row over. lq ignores the low 4 address bits (kept). MMI becomes plain halfword code; field meanings inferred from the registers. */

typedef int s32_c126598;
typedef unsigned char u8_c126598;
typedef unsigned short u16_c126598;
typedef unsigned int u32_c126598;
typedef struct {
    u16_c126598 *dst;
    s32_c126598 shift;
    s32_c126598 rows;
    s32_c126598 rows2;
    s32_c126598 stride;
    u8_c126598 *src1;
    u8_c126598 *src2;
} MC_1266D8_c126598;
static void load_1266D8_c126598(u16_c126598 *out, const u8_c126598 *s1, const u8_c126598 *s2, s32_c126598 n) {
    u8_c126598 c[32];
    s32_c126598 i;
    s1 = (const u8_c126598 *)((u32_c126598)s1 & ~15u);
    s2 = (const u8_c126598 *)((u32_c126598)s2 & ~15u);
    for (i = 0; i < 16; i++) {
        c[i] = s1[i];
        c[16 + i] = s2[i];
    }
    for (i = 0; i < 16; i++) {
        out[i] = c[n + i];
    }
}
void func_00126598(MC_1266D8_c126598 *a) {
    u8_c126598 *s1 = a->src1;
    u8_c126598 *s2 = a->src2;
    u8_c126598 *d = (u8_c126598 *)a->dst;
    s32_c126598 n = a->shift & 15;
    s32_c126598 stride = a->stride;
    s32_c126598 cnt = a->rows;
    s32_c126598 first = 1;
    u16_c126598 prev[16];
    u16_c126598 cur[16];
    s32_c126598 i;
    load_1266D8_c126598(prev, s1, s2, n);
    if (cnt == 0) {
        goto after;
    }
    for (;;) {
        do {
            s1 += stride;
            s2 += stride;
            load_1266D8_c126598(cur, s1, s2, n);
            cnt--;
            for (i = 0; i < 16; i++) {
                ((u16_c126598 *)d)[i] = (u16_c126598)(prev[i] + cur[i] + 1) >> 1;
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
