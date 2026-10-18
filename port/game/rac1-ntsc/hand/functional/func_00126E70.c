/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126FB0.c (games/rac1): func_00126FB0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI translated to plain arithmetic: 8-wide horizontal half-pel average into dst with signed round-up, two column passes of two row blocks. */

typedef short s16_c126e70;
typedef int s32_c126e70;
typedef unsigned char u8_c126e70;
typedef unsigned short u16_c126e70;
typedef unsigned int u32_c126e70;
static void rot8_func_00126FB0_c126e70(const u8_c126e70 *a, const u8_c126e70 *b, int k, u8_c126e70 *rot) {
    u8_c126e70 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void st16_func_00126FB0_c126e70(u8_c126e70 *d, const u16_c126e70 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c126e70 *)d)[i] = v[i];
}
static int gt_func_00126FB0_c126e70(u16_c126e70 v) { return (s16_c126e70)v > 0; }
void func_00126E70(u32_c126e70 *p) {
    u8_c126e70 *s1 = (u8_c126e70 *)p[5], *s2 = (u8_c126e70 *)p[6], *dst = (u8_c126e70 *)p[0];
    int k = p[1] & 15, stride = (s32_c126e70)p[4], rows, i, pass, blk;
    for (pass = 0; pass < 2; pass++) {
        rows = (s32_c126e70)p[2];
        for (blk = 0; blk < 2; blk++) {
            do {
                u8_c126e70 rot[16];
                u16_c126e70 o[8];
                rot8_func_00126FB0_c126e70(s1, s2, k, rot);
                rows--;
                s1 += stride; s2 += stride;
                for (i = 0; i < 8; i++) {
                    u16_c126e70 v = (u16_c126e70)(rot[i] + rot[i + 1] + 1) >> 1;
                    u16_c126e70 s = (u16_c126e70)(v + ((u16_c126e70 *)dst)[i]);
                    o[i] = (u16_c126e70)(s + gt_func_00126FB0_c126e70(s)) >> 1;
                }
                st16_func_00126FB0_c126e70(dst, o, 8);
                dst += stride * 2;
            } while (rows > 0);
            s1 += 0x140; s2 += 0x140;
            rows = (s32_c126e70)p[3];
            if (rows == 0) break;
        }
        s1 = (u8_c126e70 *)p[5] + 0x40; s2 = (u8_c126e70 *)p[6] + 0x40; dst = (u8_c126e70 *)p[0] + 0x80;
    }
}
