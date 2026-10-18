/* functional: high. Per-element wave/ripple setup into scratchpad (0x70000000..): copies two words, blends two floats, rotates a (sin,cos)*scale offset onto B pairs. sin/cos taken as FastSin (vcallms 0xC90) / FastCos (0xC80); VU rounding not checked. */
/* Checked on 2026-10-17 against VU0 program 28259 as the executable holds it (micro memory
   0xC80..0xD18): 0xC80 adds pi/2 to vf1.x and falls into 0xC90, which folds the angle with
   min(x, pi - x) and max(x, -pi - x) and sums x + c3 x^3 + c5 x^5 + c7 x^7 + c9 x^9 into vf1.x,
   so the two calls are exactly func_001F9FA8 (sine) and func_001F9F90 (cosine). The program
   writes only vf1..vf6, ACC and I; this routine's live registers across the calls (vf10, vf12,
   vf13, vf15, vf16, vf20) are untouched, and vf17 (0.5) is loaded but never read. The loop
   (count from +0xC, decremented before the test), the offsets read and the scratchpad writes
   were compared with the assembly and agree. */
extern float func_001F9FA8(float);
extern float func_001F9F90(float);

void func_L01_002635C8(int *p) {
    unsigned int *a = *(unsigned int **)((char *)p + 0x0);
    float *b = *(float **)((char *)p + 0x4);
    int count = *(int *)((char *)p + 0xC);
    float zscale = *(float *)((char *)p + 0x18);
    unsigned int wordA = *(unsigned int *)((char *)p + 0x30);
    float ox = *(float *)((char *)p + 0x40);
    float oy = *(float *)((char *)p + 0x44);
    float oz = *(float *)((char *)p + 0x48);
    float ow = *(float *)((char *)p + 0x4C);
    int base = *(int *)((char *)p + 0x50);
    float f = *(float *)((char *)p + 0x5C);
    unsigned int *s0 = (unsigned int *)0x70000000;
    float *s1 = (float *)0x70001000;
    float *s2 = (float *)0x70002000;
    unsigned int *s3 = (unsigned int *)0x70003000;

    do {
        unsigned int r1 = a[0];
        unsigned int r2 = a[1];
        float a2 = ((float *)a)[2];
        float a3 = ((float *)a)[3];
        unsigned int idx = ((r1 + r2) >> 16) & 0xFF;
        int ang = (int)((((idx << 6) + base) << 3) & 0xFFF);
        float angle = (float)ang * (1.0f / 4096.0f);
        float sn, cs, vx, vy;
        angle = angle * 6.2831855f;
        angle = angle - 3.1415927f;
        s0[0] = r1;
        s0[1] = r2;
        sn = func_001F9FA8(angle);
        cs = func_001F9F90(angle);
        ((float *)s0)[2] = a2 * f + a3 * (1.0f - f);
        vx = sn * zscale;
        vy = cs * zscale;
        s1[0] = (b[0] + ox) + vx;
        s1[1] = (b[1] + oy) + vy;
        s2[0] = (b[0] + oz) - vx;
        s2[1] = (b[1] + ow) - vy;
        s3[0] = wordA;
        s0 += 3;
        b += 2;
        a += 4;
        s1 += 2;
        s2 += 2;
        s3 += 1;
    } while (--count != 0);
}
