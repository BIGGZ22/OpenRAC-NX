/* functional: high. fcheck: unchecked. Sky tile visibility: for each of count tiles (32-byte stride, bounding sphere x,y,z,r in the first quadword) scales the sphere by D_0018D080[0x30].w, rotates the centre by (rows 0..2 of D_0018D080) x (rows 0..2 of D_001D9A70), and writes visibility[i] = 1 when both D_0018D080[0x40].xy * z - (|x|,|y| - D_0018D080[0x60].xy * r) have a clear sign bit, else 0 (count is assumed >= 1, as the loop is post-tested in retail too). Differential test: 60 cases agree with retail run in an EE emulator (unicorn with an R5900 MMI/VU0 instruction model). */
extern float D_0018D080[];
extern float D_001D9A70[];

static int sky_sign_set(float f) {
    union { float f; u32 u; } v;
    v.f = f;
    return (v.u >> 31) != 0;
}

void func_0022D7E0(void *tiles, s32 count, u8 *visibility) {
    const float *m = D_0018D080;
    const float *r = D_001D9A70;
    const float *t = (const float *)tiles;
    float a[9]; /* a[3 * k + j]: column j of the combined matrix row k */
    float *ap = a;
    float s = m[15];
    s32 k, j;

    /* a[k] = view rows combined with sky rotation row k */
    for (k = 0; k < 3; k++, r += 4) {
        for (j = 0; j < 3; j++) {
            *ap++ = m[j] * r[0] + m[4 + j] * r[1] + m[8 + j] * r[2];
        }
    }

    do {
        float x = t[0] * s;
        float y = t[1] * s;
        float z = t[2] * s;
        float w = t[3] * s;
        float vx = a[0] * x + a[3] * y + a[6] * z;
        float vy = a[1] * x + a[4] * y + a[7] * z;
        float vz = a[2] * x + a[5] * y + a[8] * z;
        float ex = m[16] * vz - (__builtin_fabsf(vx) - m[24] * w);
        float ey = m[17] * vz - (__builtin_fabsf(vy) - m[25] * w);

        *visibility++ = (sky_sign_set(ex) || sky_sign_set(ey)) ? 0 : 1;
        t += 8;
    } while (--count != 0);
}
