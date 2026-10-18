/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126BC0.c (games/rac1): func_00126BC0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 16-wide average of two sources into the existing dst contents ((dst + src + 1) >> 1). Byte rotation modelled explicitly. */

typedef int s32_c126a80;
typedef unsigned char u8_c126a80;
typedef unsigned short u16_c126a80;
typedef unsigned int u32_c126a80;
static void cat16_func_00126BC0_c126a80(const u8_c126a80 *a, const u8_c126a80 *b, int k, u8_c126a80 *r) {
    u8_c126a80 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_00126BC0_c126a80(u8_c126a80 *d, const u16_c126a80 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126a80 *)d)[i] = v[i];
}
void func_00126A80(u32_c126a80 *p) {
    u8_c126a80 *s1 = (u8_c126a80 *)p[5], *s2 = (u8_c126a80 *)p[6], *dst = (u8_c126a80 *)p[0];
    int k = p[1] & 15, stride = (s32_c126a80)p[4], rows = (s32_c126a80)p[2], i, first = 1;
    u8_c126a80 r[17];
    u16_c126a80 o[16];
loop:
    cat16_func_00126BC0_c126a80(s1, s2, k, r);
    for (i = 0; i < 16; i++) o[i] = (u16_c126a80)(((u16_c126a80 *)dst)[i] + r[i] + 1) >> 1;
    st16_func_00126BC0_c126a80(dst, o, 16);
    rows--;
    s1 += stride; dst += stride * 2; s2 += stride;
    if (rows > 0) goto loop;
    s1 += 0x80; s2 += 0x80;
    rows = (s32_c126a80)p[3];
    if (first && rows != 0) { first = 0; goto loop; }
}
