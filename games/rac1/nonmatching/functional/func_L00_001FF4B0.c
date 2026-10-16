/* functional: medium. Writes out = v * (a / |v|) with |v| the length of v's xyz; when that length is zero, out.xyz = 0. out.w = v.w in both cases. The vector unit's reciprocal square root and float rounding are not IEEE, so the last bits may differ from retail. */
void func_L00_001FF4B0(float *out, float *v, float a) {
    float len2 = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
    float s;

    if (len2 == 0.0f) {
        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f;
        out[3] = v[3];
        return;
    }
    s = a / sqrtf(len2);
    out[0] = v[0] * s;
    out[1] = v[1] * s;
    out[2] = v[2] * s;
    out[3] = v[3];
}
