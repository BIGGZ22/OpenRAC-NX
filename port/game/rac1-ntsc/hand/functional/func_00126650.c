/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126790.c (games/rac1): func_00126790, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI (MPEG motion compensation, 8-wide vertical half-pel): each row is 8 bytes from src1 and 8 from src2 joined and rotated right by (shift & 15), the low 8 bytes widened to halfwords; output row = (previous row + this row + 1) >> 1, one 16-byte store. Two passes (source +0x40, dest +0x80 for the second); each runs a[2] rows, then a second block of a[3] rows (source +0x140) when the low 16 bits of a[3] are nonzero (retail builds that test with a halfword shift, so only 16 bits count). MMI becomes plain halfword code; field meanings inferred from the registers. */

typedef int s32_c126650;
typedef unsigned char u8_c126650;
typedef unsigned short u16_c126650;
typedef struct {
    u16_c126650 *dst;
    s32_c126650 shift;
    s32_c126650 rows;
    s32_c126650 rows2;
    s32_c126650 stride;
    u8_c126650 *src1;
    u8_c126650 *src2;
} MC_126790_c126650;
static void load_126790_c126650(u16_c126650 *out, const u8_c126650 *s1, const u8_c126650 *s2, s32_c126650 n) {
    u8_c126650 c[16];
    s32_c126650 i;
    for (i = 0; i < 8; i++) {
        c[i] = s1[i];
        c[8 + i] = s2[i];
    }
    for (i = 0; i < 8; i++) {
        out[i] = c[(n + i) & 15];
    }
}
void func_00126650(MC_126790_c126650 *a) {
    s32_c126650 pass;
    s32_c126650 n = a->shift & 15;
    s32_c126650 stride = a->stride;
    for (pass = 0; pass < 2; pass++) {
        u8_c126650 *s1 = a->src1;
        u8_c126650 *s2 = a->src2;
        u8_c126650 *d = (u8_c126650 *)a->dst;
        s32_c126650 cnt = a->rows;
        s32_c126650 i;
        u16_c126650 prev[8];
        u16_c126650 cur[8];
        s32_c126650 second = 0;
        if (pass) {
            s1 += 0x40;
            s2 += 0x40;
            d += 0x80;
        }
        load_126790_c126650(prev, s1, s2, n);
        if (cnt != 0) {
            goto block;
        }
        for (;;) {
            s1 += 0x140;
            s2 += 0x140;
            if (second || (a->rows2 & 0xFFFF) == 0) {
                break;
            }
            second = 1;
            cnt = a->rows2;
        block:
            do {
                s1 += stride;
                s2 += stride;
                load_126790_c126650(cur, s1, s2, n);
                cnt--;
                for (i = 0; i < 8; i++) {
                    ((u16_c126650 *)d)[i] = (u16_c126650)(cur[i] + prev[i] + 1) >> 1;
                    prev[i] = cur[i];
                }
                d += stride * 2;
            } while (cnt > 0);
        }
    }
}
