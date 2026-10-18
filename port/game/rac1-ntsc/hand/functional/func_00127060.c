/* Carried by port/tools/versionmap/carry_functions.py from the decompilation of Ratchet & Clank (PAL), nonmatching/functional/func_001271A0.c (games/rac1): func_001271A0, functional C there, not matched.
 * Kind `same` in the version map (../../README.md): the two versions have the same instructions for it.
 * Names and addresses are this version's. Functional C, not matched C. */
/* functional: medium. Plain-C version of the EE multimedia-instruction code: 8-wide half-pel bilinear row filter (two doubleword sources joined and rotated by the alignment offset), averaged into an 8-halfword destination row. Two outer passes (second at src +0x40, dst +0x80), each with the main rows (record +8) and an optional continuation of the record +0xC rows. Decoded from the assembly alone; the rotation wraps within the 16 bytes exactly as qfsrv does. */

typedef unsigned char u8_71A0_c127060;
typedef unsigned short u16_71A0_c127060;
static void rowsum_71A0_c127060(unsigned int a5, unsigned int a6, int sh, u16_71A0_c127060 out[8]) {
    u8_71A0_c127060 cat[16], rot[16];
    int i;
    for (i = 0; i < 8; i++) {
        cat[i] = ((u8_71A0_c127060 *)(a5 & ~7u))[i];
        cat[8 + i] = ((u8_71A0_c127060 *)(a6 & ~7u))[i];
    }
    for (i = 0; i < 16; i++) {
        rot[i] = cat[(sh + i) & 15];
    }
    for (i = 0; i < 8; i++) {
        u8_71A0_c127060 nx = (i + 1 < 16) ? rot[i + 1] : 0;
        out[i] = (u16_71A0_c127060)(rot[i] + nx);
    }
}
static void rows_71A0_c127060(u8_71A0_c127060 **dstp, unsigned int *s5, unsigned int *s6, int stride, int sh, int n, u16_71A0_c127060 s0[8]) {
    u8_71A0_c127060 *dst = *dstp;
    u16_71A0_c127060 s1[8];
    int i;
    do {
        *s6 += stride;
        rowsum_71A0_c127060(*s5, *s6, sh, s1);
        n--;
        *s5 += stride;
        for (i = 0; i < 8; i++) {
            u16_71A0_c127060 sum = (u16_71A0_c127060)(s1[i] + s0[i]);
            short v;
            short d = ((short *)dst)[i];
            s0[i] = s1[i];
            v = (short)((u16_71A0_c127060)(sum + 2) >> 2);
            v = (short)(v + d);
            v = (short)(v + (v > 0 ? 1 : 0));
            ((u16_71A0_c127060 *)dst)[i] = (u16_71A0_c127060)((u16_71A0_c127060)v >> 1);
        }
        dst += stride * 2;
    } while (n > 0);
    *dstp = dst;
}
void func_00127060(void *arg0) {
    char *p = (char *)arg0;
    u8_71A0_c127060 *dst = (u8_71A0_c127060 *)*(unsigned int *)(p + 0x0);
    int sh = *(int *)(p + 0x4) & 15;
    int stride = *(int *)(p + 0x10);
    unsigned int s5 = *(unsigned int *)(p + 0x14);
    unsigned int s6 = *(unsigned int *)(p + 0x18);
    unsigned int flags = 1;
    u16_71A0_c127060 s0[8];
    int n;
    for (;;) {
        n = *(int *)(p + 0x8);
        flags |= 0x8000;
        rowsum_71A0_c127060(s5, s6, sh, s0);
        s5 += stride;
        if (n != 0) {
            rows_71A0_c127060(&dst, &s5, &s6, stride, sh, n, s0);
        }
        for (;;) {
            int hit = (flags & 0x8000) != 0;
            s5 += 0x140;
            n = *(int *)(p + 0xC);
            s6 += 0x140;
            flags &= 0x7FFF;
            if (hit && (n & 0xFFFF) != 0) {
                rows_71A0_c127060(&dst, &s5, &s6, stride, sh, n, s0);
                continue;
            }
            break;
        }
        s5 = *(unsigned int *)(p + 0x14) + 0x40;
        s6 = *(unsigned int *)(p + 0x18) + 0x40;
        dst = (u8_71A0_c127060 *)(*(unsigned int *)(p + 0x0) + 0x80);
        if ((flags & 1) == 0) {
            break;
        }
        flags &= 0xFFFE;
    }
}
