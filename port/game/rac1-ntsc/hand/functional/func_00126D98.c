/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126ED8.c (games/rac1): func_00126ED8, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 16-wide horizontal half-pel average into dst. Retail rounds the low 8 lanes with +1 and the high 8 lanes with (sum > 0); kept as is. Byte rotation modelled explicitly. */

typedef short s16_c126d98;
typedef int s32_c126d98;
typedef unsigned char u8_c126d98;
typedef unsigned short u16_c126d98;
typedef unsigned int u32_c126d98;
static void cat16_func_00126ED8_c126d98(const u8_c126d98 *a, const u8_c126d98 *b, int k, u8_c126d98 *r) {
    u8_c126d98 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_00126ED8_c126d98(u8_c126d98 *d, const u16_c126d98 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126d98 *)d)[i] = v[i];
}
static int gt_func_00126ED8_c126d98(u16_c126d98 v) { return (s16_c126d98)v > 0; }
void func_00126D98(u32_c126d98 *p) {
    u8_c126d98 *s1 = (u8_c126d98 *)p[5], *s2 = (u8_c126d98 *)p[6], *dst = (u8_c126d98 *)p[0];
    int k = p[1] & 15, stride = (s32_c126d98)p[4], rows = (s32_c126d98)p[2], i, first = 1;
    u8_c126d98 r[17];
    u16_c126d98 o[16];
loop:
    cat16_func_00126ED8_c126d98(s1, s2, k, r);
    rows--;
    for (i = 0; i < 16; i++) {
        u16_c126d98 v = (u16_c126d98)(r[i] + r[i + 1] + 1) >> 1;
        u16_c126d98 s = (u16_c126d98)(v + ((u16_c126d98 *)dst)[i]);
        o[i] = (i < 8) ? (u16_c126d98)(s + 1) >> 1 : (u16_c126d98)(s + gt_func_00126ED8_c126d98(s)) >> 1;
    }
    st16_func_00126ED8_c126d98(dst, o, 16);
    s1 += stride; s2 += stride;
    dst += stride * 2;
    if (rows > 0) goto loop;
    s1 += 0x80; s2 += 0x80;
    rows = (s32_c126d98)p[3];
    if (first && rows != 0) { first = 0; goto loop; }
}
