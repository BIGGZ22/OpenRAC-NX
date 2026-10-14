/* functional: medium. Scales the four floats at v by s, truncates to int, clamps each to -0x8000..0x7FFF and packs them as four 16-bit values (x lowest) into the 64-bit result. VU0 ftoi saturation modelled by clamping in float first; not checked against hardware. */
long func_L00_001FF718(float *v, float s) {
    float f[4];
    unsigned long r = 0;
    int i;
    f[0] = v[0] * s;
    f[1] = v[1] * s;
    f[2] = v[2] * s;
    f[3] = v[3] * s;
    for (i = 0; i < 4; i++) {
        int n;
        if (f[i] >= 32767.0f) n = 32767;
        else if (f[i] <= -32768.0f) n = -32768;
        else n = (int)f[i];
        r |= (unsigned long)(unsigned short)n << (16 * i);
    }
    return (long)r;
}
