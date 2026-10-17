/* functional: medium. Handwritten MMI translated to plain arithmetic: 8-wide vertical half-pel average into dst with signed round-up; pass/block flag register kept as in retail (second block test is flag bit 15 and (p[3] & 0xFFFF)). */
static void rot8_func_00126DF0(const u8 *a, const u8 *b, int k, u8 *rot) {
    u8 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void cat16_func_00126DF0(const u8 *a, const u8 *b, int k, u8 *r) {
    u8 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_00126DF0(u8 *d, const u16 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16 *)d)[i] = v[i];
}
static int gt_func_00126DF0(u16 v) { return (s16)v > 0; }
void func_00126DF0(u32 *p) {
    u8 *s1 = (u8 *)p[5], *s2 = (u8 *)p[6], *dst = (u8 *)p[0];
    int k = p[1] & 15, stride = (s32)p[4], rows, i;
    u32 f = 1;
    u8 rot[16];
    u16 prev[8], o[8];
top:
    rows = (s32)p[2];
    rot8_func_00126DF0(s1, s2, k, rot);
    f |= 0x8000;
    for (i = 0; i < 8; i++) prev[i] = rot[i];
    if (rows == 0) goto tail;
loop:
    s1 += stride; s2 += stride;
    rot8_func_00126DF0(s1, s2, k, rot);
    rows--;
    for (i = 0; i < 8; i++) {
        u16 v = (u16)(rot[i] + prev[i] + 1) >> 1;
        u16 s = (u16)(v + ((u16 *)dst)[i]);
        o[i] = (u16)(s + gt_func_00126DF0(s)) >> 1;
        prev[i] = rot[i];
    }
    st16_func_00126DF0(dst, o, 8);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    {
        int again = (f & 0x8000) != 0;
        s1 += 0x140; s2 += 0x140;
        rows = (s32)p[3];
        f &= 0x7FFF;
        if (again && (rows & 0xFFFF) != 0) goto loop;
    }
    s1 = (u8 *)p[5] + 0x40; s2 = (u8 *)p[6] + 0x40; dst = (u8 *)p[0] + 0x80;
    {
        int more = f & 1;
        f &= 0xFFFE;
        if (more) goto top;
    }
}
