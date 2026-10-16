/* Scales the vector at in (x, y, z; w copied) to length r when its length is at least r and nonzero: stores it at out and returns 1; otherwise stores nothing and returns 0. VU0: vsqrt for the length, vdiv for r / length. */
extern float sqrtf(float);
int func_L00_001FF548(float *out, float *in, float r) {
    float x = in[0];
    float y = in[1];
    float z = in[2];
    float s = (x * x + y * y) + z * z;
    float len = sqrtf(s);
    float q;
    if (len == 0.0f) {
        return 0;
    }
    if (r - len > 0.0f) {
        return 0;
    }
    q = r / len;
    out[0] = x * q;
    out[1] = y * q;
    out[2] = z * q;
    out[3] = in[3];
    return 1;
}
