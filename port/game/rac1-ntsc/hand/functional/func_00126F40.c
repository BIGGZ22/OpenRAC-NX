/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_00127080.c (games/rac1): func_00127080, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Plain-C version of the EE multimedia-instruction code: 16-wide half-pel bilinear row filter of two reference quadwords (halfword intermediates), averaged into the 16-halfword destination row. Decoded from the assembly alone. The rounding constant is 2 for the first row only and 1 afterwards, because the assembly rebuilds it as 1 after the first row (kept as in retail). Argument record: +0 dst, +4 alignment byte offset, +8 rows, +0xC rows of the second pass, +0x10 stride, +0x14 and +0x18 the two source quadword pointers. */

typedef unsigned char u8_7080_c126f40;
typedef unsigned short u16_7080_c126f40;
static void rowsum_7080_c126f40(unsigned int a5, unsigned int a6, int sh, u16_7080_c126f40 out[16]) {
    u8_7080_c126f40 q1[16], q2[16], cat[32], cur[16], nxt[16];
    int i;
    for (i = 0; i < 16; i++) {
        q1[i] = ((u8_7080_c126f40 *)(a5 & ~15u))[i];
        q2[i] = ((u8_7080_c126f40 *)(a6 & ~15u))[i];
        cat[i] = q1[i];
        cat[16 + i] = q2[i];
    }
    for (i = 0; i < 16; i++) {
        cur[i] = cat[sh + i];
    }
    for (i = 0; i < 15; i++) {
        nxt[i] = cur[i + 1];
    }
    nxt[15] = q2[sh];
    for (i = 0; i < 16; i++) {
        out[i] = (u16_7080_c126f40)(cur[i] + nxt[i]);
    }
}
void func_00126F40(void *arg0) {
    char *p = (char *)arg0;
    u8_7080_c126f40 *dst = (u8_7080_c126f40 *)*(unsigned int *)(p + 0x0);
    int sh = *(int *)(p + 0x4) & 15;
    int rows = *(int *)(p + 0x8);
    int stride = *(int *)(p + 0x10);
    unsigned int s5 = *(unsigned int *)(p + 0x14);
    unsigned int s6 = *(unsigned int *)(p + 0x18);
    unsigned int rnd = 2;
    int again = 1;
    u16_7080_c126f40 s0[16], s1[16];
    int i;
    rowsum_7080_c126f40(s5, s6, sh, s0);
    for (;;) {
        if (rows != 0) {
            do {
                s5 += stride;
                s6 += stride;
                rowsum_7080_c126f40(s5, s6, sh, s1);
                rows--;
                for (i = 0; i < 16; i++) {
                    u16_7080_c126f40 v = (u16_7080_c126f40)(s0[i] + s1[i]);
                    u16_7080_c126f40 d = ((u16_7080_c126f40 *)dst)[i];
                    s0[i] = s1[i];
                    v = (u16_7080_c126f40)((u16_7080_c126f40)(v + rnd) >> 2);
                    v = (u16_7080_c126f40)(v + d);
                    rnd = 1;
                    v = (u16_7080_c126f40)((u16_7080_c126f40)(v + 1) >> 1);
                    ((u16_7080_c126f40 *)dst)[i] = v;
                }
                dst += stride * 2;
            } while (rows > 0);
        }
        s5 += 0x80;
        s6 += 0x80;
        rows = *(int *)(p + 0xC);
        if (!again || rows == 0) {
            break;
        }
        again = 0;
    }
}
