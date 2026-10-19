/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Normalises the xyz of the vector at arg1 into arg0 and writes w = 0, as the VU0 sequence does. The two stores at the start write leftover VU registers that the final store overwrites, so they are dropped. A zero vector gives zero (retail's divide by zero is multiplied by zero). The VU0 square root and divide may differ from the host in the last bit. */
void func_001252A0(void *arg0, void *arg1) {
    float *out = (float *)arg0;
    const float *in = (const float *)arg1;
    float dot = in[0] * in[0] + in[1] * in[1] + in[2] * in[2];
    float len = __builtin_sqrtf(__builtin_fabsf(dot));
    float inv = len > 0.0f ? 1.0f / len : 0.0f;

    out[0] = in[0] * inv;
    out[1] = in[1] * inv;
    out[2] = in[2] * inv;
    out[3] = 0.0f;
}
