/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00126910.c (games/rac1): func_00126910, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Handwritten MMI (MPEG motion compensation) translated to plain halfword arithmetic: 8-wide half-pel horizontal average of two sources into dst, two column passes of two row blocks. Register-level details (qfsrv byte rotation, zero fill) are modelled byte by byte; the addi overflow trap cannot occur. */

typedef int s32_c1267d0;
typedef unsigned char u8_c1267d0;
typedef unsigned short u16_c1267d0;
typedef unsigned int u32_c1267d0;
static void rot8_func_00126910_c1267d0(const u8_c1267d0 *a, const u8_c1267d0 *b, int k, u8_c1267d0 *rot) {
    u8_c1267d0 buf[16];
    int i;
    for (i = 0; i < 8; i++) { buf[i] = a[i]; buf[8 + i] = b[i]; }
    for (i = 0; i < 16; i++) rot[i] = buf[(i + k) & 15];
}
static void st16_func_00126910_c1267d0(u8_c1267d0 *d, const u16_c1267d0 *v, int n) {
    int i;
    for (i = 0; i < n; i++) ((u16_c1267d0 *)d)[i] = v[i];
}
void func_001267D0(u32_c1267d0 *p) {
    u8_c1267d0 *s1 = (u8_c1267d0 *)p[5], *s2 = (u8_c1267d0 *)p[6], *dst = (u8_c1267d0 *)p[0];
    int k = p[1] & 15, stride = (s32_c1267d0)p[4], rows, blk, pass;
    for (pass = 0; pass < 2; pass++) {
        rows = (s32_c1267d0)p[2];
        for (blk = 0; blk < 2; blk++) {
            do {
                u8_c1267d0 rot[16];
                u16_c1267d0 o[8];
                int i;
                rot8_func_00126910_c1267d0(s1, s2, k, rot);
                for (i = 0; i < 8; i++) o[i] = (u16_c1267d0)(rot[i] + rot[i + 1] + 1) >> 1;
                rows--;
                s1 += stride; s2 += stride;
                st16_func_00126910_c1267d0(dst, o, 8);
                dst += stride * 2;
            } while (rows > 0);
            s1 += 0x140; s2 += 0x140;
            rows = (s32_c1267d0)p[3];
            if (rows == 0) break;
        }
        s1 = (u8_c1267d0 *)p[5] + 0x40; s2 = (u8_c1267d0 *)p[6] + 0x40; dst = (u8_c1267d0 *)p[0] + 0x80;
    }
}
