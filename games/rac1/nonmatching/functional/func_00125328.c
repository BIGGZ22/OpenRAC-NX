/* functional: high. Scales the four floats at arg1 by the float argument and writes them to arg0 (lqc2, vmulx.xyzw, sqc2). Entry 0x125328 sits inside func_00125300's assembly but is a complete leaf routine with its own jr $31 and reads only its arguments; func_L00_0025BCF8 calls it with (dst, src, scale). */
void func_00125328(void *arg0, void *arg1, float arg2) {
    float *out = (float *)arg0;
    const float *in = (const float *)arg1;
    int i;

    for (i = 0; i < 4; i++) {
        out[i] = in[i] * arg2;
    }
}
