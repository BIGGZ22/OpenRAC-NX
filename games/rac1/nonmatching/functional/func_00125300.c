/* functional: medium. Scales the four floats at arg1 by the float argument and writes them to arg0. The VU0 code first stores leftover VU registers to arg0+0x30 and arg0+0x00 (the latter is overwritten by the final store); those leftovers are not reproducible and are dropped, so arg0+0x30 keeps its old contents here. */
void func_00125300(void *arg0, void *arg1, float arg2) {
    float *out = (float *)arg0;
    const float *in = (const float *)arg1;
    int i;

    for (i = 0; i < 4; i++) {
        out[i] = in[i] * arg2;
    }
}
