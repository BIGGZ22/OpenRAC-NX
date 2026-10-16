/* functional: medium. Reads the 8 bytes at obj+0x38 (b0, b1, b2); table D_L00_0017FFC0 has 64-byte entries whose first quad is a colour/vector. If b2 == 0 copies entry b0's quad to out; else out.xyz = e0.xyz*(1-t) + e1.xyz*t with t = b2/256 (e0 = entry b0, e1 = entry b1) and out.w = e0.w. Joined with func_L00_00251438; VU rounding (ACC then madd) written as separate float operations; not run. */
extern char D_L00_0017FFC0_513F8[] __asm__("D_L00_0017FFC0");

void func_L00_002513F8(char *obj, float *out) {
    unsigned long long v = *(unsigned long long *)(obj + 0x38);
    unsigned int b0 = (unsigned int)(v & 0xFF);
    unsigned int b1 = (unsigned int)((v >> 8) & 0xFF);
    unsigned int b2 = (unsigned int)((v >> 16) & 0xFF);
    float *e0 = (float *)(D_L00_0017FFC0_513F8 + (b0 << 6));

    if ((int)b2 <= 0) {
        out[0] = e0[0];
        out[1] = e0[1];
        out[2] = e0[2];
        out[3] = e0[3];
    } else {
        float *e1 = (float *)(D_L00_0017FFC0_513F8 + (b1 << 6));
        float t = (float)(int)(b2 << 4) * (1.0f / 4096.0f);
        float w = 1.0f - t;
        float x = e0[0] * w;
        float y = e0[1] * w;
        float z = e0[2] * w;
        float x2 = e1[0] * t;
        float y2 = e1[1] * t;
        float z2 = e1[2] * t;
        out[0] = x + x2;
        out[1] = y + y2;
        out[2] = z + z2;
        out[3] = e0[3];
    }
}
