extern float D_L00_00182B70[];
/* Two scaled outputs from a 16-byte table entry T[4m..4m+3] (m a mode 0..18;
   mode -1 reads D_0015EE84, other out-of-range modes give 0). Mode 6 with k >= 100
   uses fixed words of the entry and constants. Retail's tail to func_L00_0024B2A4
   is written inline. Scale is 2^-9. */
void func_L00_0024B200(float *p1, float *p2, int k, float t, float u) {
    float *T = D_L00_00182B70;
    float s = 0.001953125f;
    int seven = 0;
    int m;
    float x;
    float y;
    float z;
    float w;
    if (k >= 100) {
        k -= 100;
        seven = 1;
    }
    m = k;
    if (m == -1) {
        m = D_0015EE84_m;
    }
    if (m < 0) {
        m = 0;
    }
    if (m >= 19) {
        m = 0;
    }
    if (m == 6 && seven) {
        x = T[27] * u;
        y = x + 1053.0f;
        *p1 = y * s;
        z = T[25] * t;
        w = 740.0f - z;
        *p2 = w * s;
        return;
    }
    x = T[4 * m + 1] * t;
    y = T[4 * m] + x;
    *p1 = y * s;
    z = T[4 * m + 3] * u;
    w = T[4 * m + 2] + z;
    *p2 = w * s;
}
