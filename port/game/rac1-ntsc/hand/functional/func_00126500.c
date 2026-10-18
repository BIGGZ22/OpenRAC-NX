/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126640.c (games/rac1): func_00126640, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI (MPEG motion compensation, 8-wide copy without averaging): per row, joins 8 bytes of src1 and 8 of src2, rotates the 16 bytes right by (shift & 15), and widens the low 8 bytes to 8 halfwords written as one 16-byte store. Two passes (columns 0 and +0x40 source / +0x80 dest); each pass runs rows (a[2]) rows, then a second block of a[3] rows if a[3] is nonzero. MMI becomes plain byte/halfword code; the field meanings are inferred from the registers. */

typedef int s32_c126500;
typedef unsigned char u8_c126500;
typedef unsigned short u16_c126500;
typedef struct {
    u16_c126500 *dst;
    s32_c126500 shift;
    s32_c126500 rows;
    s32_c126500 rows2;
    s32_c126500 stride;
    u8_c126500 *src1;
    u8_c126500 *src2;
} MC_126640_c126500;
static void row_126640_c126500(u16_c126500 *d, const u8_c126500 *s1, const u8_c126500 *s2, s32_c126500 n) {
    u8_c126500 c[16];
    s32_c126500 i;
    for (i = 0; i < 8; i++) {
        c[i] = s1[i];
        c[8 + i] = s2[i];
    }
    for (i = 0; i < 8; i++) {
        d[i] = c[(n + i) & 15];
    }
}
void func_00126500(MC_126640_c126500 *a) {
    s32_c126500 pass;
    s32_c126500 n = a->shift & 15;
    s32_c126500 stride = a->stride;
    for (pass = 0; pass < 2; pass++) {
        u8_c126500 *s1 = a->src1;
        u8_c126500 *s2 = a->src2;
        u8_c126500 *d = (u8_c126500 *)a->dst;
        s32_c126500 first = 1;
        s32_c126500 cnt = a->rows;
        if (pass) {
            s1 += 0x40;
            s2 += 0x40;
            d += 0x80;
        }
        for (;;) {
            do {
                row_126640_c126500((u16_c126500 *)d, s1, s2, n);
                cnt--;
                s1 += stride;
                d += stride * 2;
                s2 += stride;
            } while (cnt > 0);
            s1 += 0x140;
            s2 += 0x140;
            cnt = a->rows2;
            if (!first || cnt == 0) {
                break;
            }
            first = 0;
        }
    }
}
