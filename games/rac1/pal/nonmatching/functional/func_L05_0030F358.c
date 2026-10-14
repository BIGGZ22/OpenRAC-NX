/* functional: medium. Draw callback of the Rilgar gate glow (moby a0): builds two sets of 5 GIF packets (0x90 bytes each, 4 quad vertices with z raised by ring number, 4 colours, 4 uv pairs, tag words) on the stack from the tables at D_L05_0020BA80/BAA0/BAE0 with colours blended by a sine of the frame counter, then for every moby of class 0x346 listed for this level (state 1) transforms all 10 packets with func_L00_001FD1D8 using the moby's matrix and position. Not checked on real calls. */
typedef int U128_0F358 __attribute__((mode(TI)));
extern int D_L05_0015F6B0_0F358 __asm__("D_L05_0015F6B0");
extern int D_L05_00161DFC_0F358 __asm__("D_L05_00161DFC");
extern int D_L05_00161E00_0F358 __asm__("D_L05_00161E00");
extern int D_L05_00161E04_0F358 __asm__("D_L05_00161E04");
extern int D_L05_00161E08_0F358 __asm__("D_L05_00161E08");
extern int D_L05_00161E30_0F358 __asm__("D_L05_00161E30");
extern int D_L05_00161E34_0F358 __asm__("D_L05_00161E34");
extern int D_L05_00161E38_0F358 __asm__("D_L05_00161E38");
extern int D_L05_00161E3C_0F358 __asm__("D_L05_00161E3C");
extern int D_L05_00161E58_0F358 __asm__("D_L05_00161E58");
extern float D_L05_0020BA80_0F358[] __asm__("D_L05_0020BA80");
extern float D_L05_0020BAA0_0F358[] __asm__("D_L05_0020BAA0");
extern float D_L05_0020BAE0_0F358[] __asm__("D_L05_0020BAE0");
extern short *D_L05_001AC040_0F358[] __asm__("D_L05_001AC040");
extern char *D_L05_00160098_0F358 __asm__("D_L05_00160098");
extern float func_001FA888_0F358(int) __asm__("func_001FA888");
extern float func_001F9FA8_0F358(float) __asm__("func_001F9FA8");
extern int func_001FA8A8_0F358(int, int, float) __asm__("func_001FA8A8");
extern float func_001FA748_0F358(float, float) __asm__("func_001FA748");
extern long func_001F4868_0F358(int) __asm__("func_001F4868");
extern void func_001FA460_0F358(void *, void *) __asm__("func_001FA460");
extern void func_L00_001FD1D8_0F358(void *, void *, int) __asm__("func_L00_001FD1D8");

void func_L05_0030F358_impl(char *moby) __asm__("func_L05_0030F358");
void func_L05_0030F358_impl(char *moby) {
    unsigned char buf[0x5E0] __attribute__((aligned(16)));
    float ang;
    float t;
    int i, j, k;
    int colA, colB;
    short *e;
    char *rec;
    float *mat = (float *)(buf + 0x5A0);

    ang = func_001FA888_0F358(D_L05_0015F6B0_0F358 & 0x3F) / 63.0f * 6.28318f;
    for (i = 0; i < 5; i++) {
        t = (func_001F9FA8_0F358(ang) + 1.0f) * 0.5f;
        colA = func_001FA8A8_0F358(D_L05_00161DFC_0F358, D_L05_00161E00_0F358, t);
        colB = func_001FA8A8_0F358(D_L05_00161E04_0F358, D_L05_00161E08_0F358, t);
        ang = func_001FA748_0F358(ang, 1.2566371f);
        for (j = 0; j < 2; j++) {
            unsigned char *R = buf + j * 0x2D0 + i * 0x90;
            long tex = func_001F4868_0F358(D_L05_00161E58_0F358);
            long gs = (long)D_L05_00161E30_0F358 | ((long)D_L05_00161E34_0F358 << 2) |
                           ((long)D_L05_00161E38_0F358 << 4) | ((long)D_L05_00161E3C_0F358 << 6) |
                           ((long)0x8000 << 24);
            *(long *)(R + 0x88) = gs;
            *(long *)(R + 0x78) = tex;
            *(long *)(R + 0x80) = 0xFF9000000260L;
            *(long *)(R + 0x70) = 0;
            for (k = 0; k < 4; k++) {
                float *q = (float *)(R + 16 * k);
                float *src;
                *(float *)(R + 0x50 + 8 * k) = D_L05_0020BA80_0F358[2 * k];
                *(float *)(R + 0x54 + 8 * k) = D_L05_0020BA80_0F358[2 * k + 1];
                if (j != 0) {
                    *(int *)(R + 0x40 + 4 * k) = colA;
                    src = D_L05_0020BAA0_0F358 + 4 * k;
                } else {
                    *(int *)(R + 0x40 + 4 * k) = colB;
                    src = D_L05_0020BAE0_0F358 + 4 * k;
                }
                *(U128_0F358 *)q = *(U128_0F358 *)src;
                q[2] = q[2] + (float)(i + 1);
            }
        }
    }
    e = D_L05_001AC040_0F358[*(unsigned char *)(moby + 0x21)];
    do {
        char *o = D_L05_00160098_0F358 + ((*(unsigned short *)e & 0x7FFF) << 8);
        if (*(short *)(o + 0xA6) == 0x346 && *(unsigned char *)(o + 0x20) == 1) {
            char *o2;
            func_001FA460_0F358(mat, o + 0xC0);
            o2 = D_L05_00160098_0F358 + ((*(unsigned short *)e & 0x7FFF) << 8);
            *(U128_0F358 *)(mat + 12) = *(U128_0F358 *)(o2 + 0x10);
            mat[15] = 1.0f;
            rec = (char *)buf;
            for (i = 0; i < 5; i++) {
                func_L00_001FD1D8_0F358(buf + i * 0x90, mat, 0);
                func_L00_001FD1D8_0F358(buf + 0x2D0 + i * 0x90, mat, 0);
            }
        }
        e++;
    } while (*(short *)(e - 1) >= 0);
    (void)rec;
}
