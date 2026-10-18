/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001269C8.c (games/rac1): func_001269C8, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 16-wide 2D half-pel (four-pixel average, +2 >>2) of two sources into dst. Byte rotation modelled explicitly; the addi trap cannot occur. */

typedef int s32_c126888;
typedef unsigned char u8_c126888;
typedef unsigned short u16_c126888;
typedef unsigned int u32_c126888;
static void cat16_func_001269C8_c126888(const u8_c126888 *a, const u8_c126888 *b, int k, u8_c126888 *r) {
    u8_c126888 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_001269C8_c126888(u8_c126888 *d, const u16_c126888 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126888 *)d)[i] = v[i];
}
void func_00126888(u32_c126888 *p) {
    u8_c126888 *s1 = (u8_c126888 *)p[5], *s2 = (u8_c126888 *)p[6], *dst = (u8_c126888 *)p[0];
    int k = p[1] & 15, stride = (s32_c126888)p[4], rows = (s32_c126888)p[2], i, first = 1;
    u8_c126888 r[17];
    u16_c126888 prev[16], o[16];
    cat16_func_001269C8_c126888(s1, s2, k, r);
    for (i = 0; i < 16; i++) prev[i] = r[i] + r[i + 1];
    if (rows != 0) goto loop;
    goto tail;
loop:
    s1 += stride; s2 += stride;
    cat16_func_001269C8_c126888(s1, s2, k, r);
    rows--;
    for (i = 0; i < 16; i++) {
        u16_c126888 cur = r[i] + r[i + 1];
        o[i] = (u16_c126888)(prev[i] + cur + 2) >> 2;
        prev[i] = cur;
    }
    st16_func_001269C8_c126888(dst, o, 16);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    s1 += 0x80; s2 += 0x80;
    rows = (s32_c126888)p[3];
    if (first && rows != 0) { first = 0; goto loop; }
    first = 0;
}
