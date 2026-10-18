/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126AC0.c (games/rac1): func_00126AC0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 8-wide 2D half-pel average (+2 >>2) of two sources into dst, two column passes of two row blocks; the pass/block flag register is kept as in retail (the second block test is (flag bit 15) and (p[3] & 0xFFFF)). */

typedef int s32_c126980;
typedef unsigned char u8_c126980;
typedef unsigned short u16_c126980;
typedef unsigned int u32_c126980;
static void rot8_func_00126AC0_c126980(const u8_c126980 *a, const u8_c126980 *b, int k, u8_c126980 *rot) {
    u8_c126980 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void st16_func_00126AC0_c126980(u8_c126980 *d, const u16_c126980 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126980 *)d)[i] = v[i];
}
void func_00126980(u32_c126980 *p) {
    u8_c126980 *s1 = (u8_c126980 *)p[5], *s2 = (u8_c126980 *)p[6], *dst = (u8_c126980 *)p[0];
    int k = p[1] & 15, stride = (s32_c126980)p[4], rows, i;
    u32_c126980 f = 1;
    u8_c126980 rot[16];
    u16_c126980 prev[8], o[8];
top:
    rows = (s32_c126980)p[2];
    rot8_func_00126AC0_c126980(s1, s2, k, rot);
    s1 += stride;
    f |= 0x8000;
    for (i = 0; i < 8; i++) prev[i] = rot[i] + rot[i + 1];
    if (rows == 0) goto tail;
loop:
    s2 += stride;
    rot8_func_00126AC0_c126980(s1, s2, k, rot);
    rows--;
    s1 += stride;
    for (i = 0; i < 8; i++) {
        u16_c126980 cur = rot[i] + rot[i + 1];
        o[i] = (u16_c126980)(cur + prev[i] + 2) >> 2;
        prev[i] = cur;
    }
    st16_func_00126AC0_c126980(dst, o, 8);
    dst += stride * 2;
    if (rows > 0) goto loop;
tail:
    {
        int again = (f & 0x8000) != 0;
        s1 += 0x140; s2 += 0x140;
        rows = (s32_c126980)p[3];
        f &= 0x7FFF;
        if (again && (rows & 0xFFFF) != 0) goto loop;
    }
    s1 = (u8_c126980 *)p[5] + 0x40; s2 = (u8_c126980 *)p[6] + 0x40; dst = (u8_c126980 *)p[0] + 0x80;
    {
        int more = f & 1;
        f &= 0xFFFE;
        if (more) goto top;
    }
}
