/* functional: high. fcheck: unchecked. Sky vertex transform: for count (>= 1; the loop is post-tested) 8-byte vertices at arg0 (s16 x,y,z, u8 tag at +6), transforms (x,y,z,1) by the sky matrix D_001D9A70 times the camera matrix at D_00187040+0x100, clips xyz against |w| (VU clip flags +x,-x,+y,-y,-z kept: mask 0x2F), divides xyz by w, maps to screen with D_0018CE00+0x190 (xy scaled by 0.25) and +0x1A0, and writes a 16-byte record per vertex at arg1: three 12.4 fixed-point ints (vftoi4: truncated, saturated), then tag, 0, 0, clip flags. Returns the AND of all clip flags (0x3F seed); 0 means draw. arg3 is unused. Like retail it reads one vertex past the end (not used). Differential test: 80 cases with exactly representable (dyadic) matrices agree bit for bit with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model, vclipw/clip-flag model added); with real rotation matrices 38 of 60 cases agree bit for bit and the rest differ only in the 12.4 screen coordinates by VU0 multiply-add vs FPU rounding (mostly z, by 1-8 sixteenths; larger only for vertices near w = 0, which are clipped). */
extern float D_00187040[];
extern float D_0018CE00[];
extern float D_001D9A70[];

static s32 sky_ftoi4(float f) {
    s32 i;

    f = f * 16.0f;
    if (f >= 2147483647.0f) {
        return 0x7FFFFFFF;
    }
    if (f <= -2147483648.0f) {
        return (s32)0x80000000;
    }
    i = (s32)f;
    /* truncate toward zero even where the float-to-int conversion rounds */
    if (f >= 0.0f) {
        if ((float)i > f) {
            i--;
        }
    } else if ((float)i < f) {
        i++;
    }
    return i;
}

s32 func_0022D2AC(s32 arg0, s32 arg1, s32 arg2, s32 arg3) {
    const float *v = D_00187040 + 0x40; /* camera matrix at +0x100 */
    const float *s = D_001D9A70;
    const s16 *in = (const s16 *)arg0;
    u8 *out = (u8 *)arg1;
    float c[16];
    float *cp = c;
    float sx = D_0018CE00[0x64] * 0.25f;
    float sy = D_0018CE00[0x65] * 0.25f;
    float sz = D_0018CE00[0x66];
    const float *o = D_0018CE00 + 0x68;
    s32 result = 0x3F;
    s32 k, j;

    /* c = sky rows times camera matrix */
    for (k = 0; k < 4; k++, s += 4) {
        for (j = 0; j < 4; j++) {
            *cp++ = v[j] * s[0] + v[4 + j] * s[1] + v[8 + j] * s[2] + v[12 + j] * s[3];
        }
    }

    do {
        float x = (float)in[0];
        float y = (float)in[1];
        float z = (float)in[2];
        u8 tag = ((const u8 *)in)[6];
        float px = c[0] * x + c[4] * y + c[8] * z + c[12];
        float py = c[1] * x + c[5] * y + c[9] * z + c[13];
        float pz = c[2] * x + c[6] * y + c[10] * z + c[14];
        float pw = c[3] * x + c[7] * y + c[11] * z + c[15];
        float aw = __builtin_fabsf(pw);
        float q = 1.0f / pw;
        s32 flags = 0;
        s32 ix, iy, iz;

        if (px > aw) flags |= 0x01;
        if (px < -aw) flags |= 0x02;
        if (py > aw) flags |= 0x04;
        if (py < -aw) flags |= 0x08;
        if (pz < -aw) flags |= 0x20;

        ix = sky_ftoi4(px * q * sx + o[0]);
        iy = sky_ftoi4(py * q * sy + o[1]);
        iz = sky_ftoi4(pz * q * sz + o[2]);
        ((s32 *)out)[0] = ix;
        ((s32 *)out)[1] = iy;
        ((s32 *)out)[2] = iz;
        out[12] = tag;
        out[13] = 0;
        out[14] = 0;
        out[15] = (u8)flags;
        result &= flags;
        in += 4;
        out += 16;
    } while (--arg2 != 0);

    return result;
}
