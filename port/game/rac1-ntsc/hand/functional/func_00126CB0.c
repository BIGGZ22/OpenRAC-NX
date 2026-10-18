/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126DF0.c (games/rac1): func_00126DF0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 8-wide vertical half-pel average into dst with signed round-up; pass/block flag register kept as in retail (second block test is flag bit 15 and (p[3] & 0xFFFF)). */

typedef short s16_c126cb0;
typedef int s32_c126cb0;
typedef unsigned char u8_c126cb0;
typedef unsigned short u16_c126cb0;
typedef unsigned int u32_c126cb0;
static void rot8_func_00126DF0_c126cb0(const u8_c126cb0 *a, const u8_c126cb0 *b, int k, u8_c126cb0 *rot) {
    u8_c126cb0 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void st16_func_00126DF0_c126cb0(u8_c126cb0 *d, const u16_c126cb0 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126cb0 *)d)[i] = v[i];
}
static int gt_func_00126DF0_c126cb0(u16_c126cb0 v) { return (s16_c126cb0)v > 0; }
void func_00126CB0(u32_c126cb0 *p) {
    u8_c126cb0 *s1 = (u8_c126cb0 *)p[5], *s2 = (u8_c126cb0 *)p[6], *dst = (u8_c126cb0 *)p[0];
    int k = p[1] & 15, stride = (s32_c126cb0)p[4], rows, i;
    u32_c126cb0 f = 1;
    u8_c126cb0 rot[16];
    u16_c126cb0 prev[8], o[8];
top:
    rows = (s32_c126cb0)p[2];
    rot8_func_00126DF0_c126cb0(s1, s2, k, rot);
    f |= 0x8000;
    for (i = 0; i < 8; i++) prev[i] = rot[i];
    if (rows == 0) goto tail;
loop:
    s1 += stride; s2 += stride;
    rot8_func_00126DF0_c126cb0(s1, s2, k, rot);
    rows--;
    for (i = 0; i < 8; i++) {
        u16_c126cb0 v = (u16_c126cb0)(rot[i] + prev[i] + 1) >> 1;
        u16_c126cb0 s = (u16_c126cb0)(v + ((u16_c126cb0 *)dst)[i]);
        o[i] = (u16_c126cb0)(s + gt_func_00126DF0_c126cb0(s)) >> 1;
        prev[i] = rot[i];
    }
    st16_func_00126DF0_c126cb0(dst, o, 8);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    {
        int again = (f & 0x8000) != 0;
        s1 += 0x140; s2 += 0x140;
        rows = (s32_c126cb0)p[3];
        f &= 0x7FFF;
        if (again && (rows & 0xFFFF) != 0) goto loop;
    }
    s1 = (u8_c126cb0 *)p[5] + 0x40; s2 = (u8_c126cb0 *)p[6] + 0x40; dst = (u8_c126cb0 *)p[0] + 0x80;
    {
        int more = f & 1;
        f &= 0xFFFE;
        if (more) goto top;
    }
}
