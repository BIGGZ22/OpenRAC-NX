/* Half-angle quaternion of angle a for axis n (VU0 code). Each lane is taken from the
   cos and sin of a: R = sqrt(|2 cos a| + 2), q = -sin a / R, h = 0.5 R. Lane x holds q when
   cos a >= 0 (w holds h), otherwise x holds h and w holds q. n = 0 writes x and w, n = 1 moves
   x to y, n >= 2 moves x to z; the other lanes are zero. Writes four floats to out. */
extern float func_001F9FA8(float);
extern float func_001F9F90(float);
extern float sqrtf(float);
extern float fabsf(float);

void func_L00_001FFED8(float a, float *out, int n) {
    union { float f; int i; } u;
    float s = func_001F9FA8(a);
    float ns = -s;
    float c = func_001F9F90(a);
    float c2 = c + c;
    float r = sqrtf(fabsf(c2) + 2.0f);
    float q = ns / r;
    float h = 0.5f * r;
    float x, w;

    u.f = c2;
    if (u.i >= 0) {
        x = 0.0f + q;
        w = h;
    } else {
        x = h;
        w = 0.0f + q;
    }
    if (n == 0) {
        out[0] = x;
        out[1] = 0.0f;
        out[2] = 0.0f;
        out[3] = w;
    } else if (n == 1) {
        out[0] = 0.0f;
        out[1] = 0.0f + x;
        out[2] = 0.0f;
        out[3] = w;
    } else {
        out[0] = 0.0f;
        out[1] = 0.0f;
        out[2] = 0.0f + x;
        out[3] = w;
    }
}
