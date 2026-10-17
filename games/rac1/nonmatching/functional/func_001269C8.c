/* functional: medium. Handwritten MMI translated to plain arithmetic: 16-wide 2D half-pel (four-pixel average, +2 >>2) of two sources into dst. Byte rotation modelled explicitly; the addi trap cannot occur. */
static void rot8_func_001269C8(const u8 *a, const u8 *b, int k, u8 *rot) {
    u8 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void cat16_func_001269C8(const u8 *a, const u8 *b, int k, u8 *r) {
    u8 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_001269C8(u8 *d, const u16 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16 *)d)[i] = v[i];
}
static int gt_func_001269C8(u16 v) { return (s16)v > 0; }
void func_001269C8(u32 *p) {
    u8 *s1 = (u8 *)p[5], *s2 = (u8 *)p[6], *dst = (u8 *)p[0];
    int k = p[1] & 15, stride = (s32)p[4], rows = (s32)p[2], i, first = 1;
    u8 r[17];
    u16 prev[16], o[16];
    cat16_func_001269C8(s1, s2, k, r);
    for (i = 0; i < 16; i++) prev[i] = r[i] + r[i + 1];
    if (rows != 0) goto loop;
    goto tail;
loop:
    s1 += stride; s2 += stride;
    cat16_func_001269C8(s1, s2, k, r);
    rows--;
    for (i = 0; i < 16; i++) {
        u16 cur = r[i] + r[i + 1];
        o[i] = (u16)(prev[i] + cur + 2) >> 2;
        prev[i] = cur;
    }
    st16_func_001269C8(dst, o, 16);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    s1 += 0x80; s2 += 0x80;
    rows = (s32)p[3];
    if (first && rows != 0) { first = 0; goto loop; }
    first = 0;
}
