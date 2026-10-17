/* functional: medium. Handwritten MMI translated to plain arithmetic: 16-wide vertical half-pel average into dst. Retail rounds the low 8 lanes with +1 and the high 8 lanes with (sum > 0); kept as is. Byte rotation modelled explicitly. */
static void rot8_func_00126D10(const u8 *a, const u8 *b, int k, u8 *rot) {
    u8 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void cat16_func_00126D10(const u8 *a, const u8 *b, int k, u8 *r) {
    u8 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_00126D10(u8 *d, const u16 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16 *)d)[i] = v[i];
}
static int gt_func_00126D10(u16 v) { return (s16)v > 0; }
void func_00126D10(u32 *p) {
    u8 *s1 = (u8 *)p[5], *s2 = (u8 *)p[6], *dst = (u8 *)p[0];
    int k = p[1] & 15, stride = (s32)p[4], rows = (s32)p[2], i, first = 1;
    u8 r[17];
    u16 prev[16], o[16];
    cat16_func_00126D10(s1, s2, k, r);
    for (i = 0; i < 16; i++) prev[i] = r[i];
    if (rows == 0) goto tail;
loop:
    s1 += stride; s2 += stride;
    cat16_func_00126D10(s1, s2, k, r);
    rows--;
    for (i = 0; i < 16; i++) {
        u16 v = (u16)(prev[i] + r[i] + 1) >> 1;
        u16 s = (u16)(v + ((u16 *)dst)[i]);
        o[i] = (i < 8) ? (u16)(s + 1) >> 1 : (u16)(s + gt_func_00126D10(s)) >> 1;
        prev[i] = r[i];
    }
    st16_func_00126D10(dst, o, 16);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    s1 += 0x80; s2 += 0x80;
    rows = (s32)p[3];
    if (first && rows != 0) { first = 0; goto loop; }
}
