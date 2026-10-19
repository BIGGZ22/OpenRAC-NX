/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Handwritten MMI translated to plain arithmetic: 8-wide horizontal half-pel average into dst with signed round-up, two column passes of two row blocks. */
static void rot8_func_00126FB0(const u8 *a, const u8 *b, int k, u8 *rot) {
    u8 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void cat16_func_00126FB0(const u8 *a, const u8 *b, int k, u8 *r) {
    u8 c[32];
    int i;
    for (i = 0; i < 16; i++) { c[i] = a[i]; c[16 + i] = b[i]; }
    for (i = 0; i < 17; i++) r[i] = c[i + k];
}
static void st16_func_00126FB0(u8 *d, const u16 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16 *)d)[i] = v[i];
}
static int gt_func_00126FB0(u16 v) { return (s16)v > 0; }
void func_00126FB0(u32 *p) {
    u8 *s1 = (u8 *)p[5], *s2 = (u8 *)p[6], *dst = (u8 *)p[0];
    int k = p[1] & 15, stride = (s32)p[4], rows, i, pass, blk;
    for (pass = 0; pass < 2; pass++) {
        rows = (s32)p[2];
        for (blk = 0; blk < 2; blk++) {
            do {
                u8 rot[16];
                u16 o[8];
                rot8_func_00126FB0(s1, s2, k, rot);
                rows--;
                s1 += stride; s2 += stride;
                for (i = 0; i < 8; i++) {
                    u16 v = (u16)(rot[i] + rot[i + 1] + 1) >> 1;
                    u16 s = (u16)(v + ((u16 *)dst)[i]);
                    o[i] = (u16)(s + gt_func_00126FB0(s)) >> 1;
                }
                st16_func_00126FB0(dst, o, 8);
                dst += stride * 2;
            } while (rows > 0);
            s1 += 0x140; s2 += 0x140;
            rows = (s32)p[3];
            if (rows == 0) break;
        }
        s1 = (u8 *)p[5] + 0x40; s2 = (u8 *)p[6] + 0x40; dst = (u8 *)p[0] + 0x80;
    }
}
