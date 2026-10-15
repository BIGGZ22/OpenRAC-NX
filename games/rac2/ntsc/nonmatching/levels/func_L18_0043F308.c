/* Near miss from src/levels/18_damosel/L18_00433460.c: the build refuses this body at its
   place — 280 bytes, expected 288. Kept as a starting point;
   not linked, not counted. */

/* Standalone declarations recovered from src/levels/18_damosel/L18_00433460.c. */
typedef int TI128 __attribute__((mode(TI)));

void func_L18_0043F308(float *out, float *p0, float *p1, float *p2, float *p3, float t) {
    float t2, t3;
    float a, b, c, d, e1, e2, e3, e;

    if (t == 0.0f) {
        *(TI128 *)out = *(TI128 *)p0;
        return;
    }
    if (t == 1.0f) {
        *(TI128 *)out = *(TI128 *)p1;
        return;
    }
    t2 = t * t;
    t3 = t2 * t;
    a = p0[0];
    b = p1[0];
    d = p3[0];
    c = p2[0];
    e1 = c - a;
    e2 = d - b;
    e3 = b - c;
    e = e2 - e1;
    out[0] = e * t3 + (e1 - e) * t2 + e3 * t + a;
    a = p0[1];
    b = p1[1];
    d = p3[1];
    c = p2[1];
    e1 = c - a;
    e2 = d - b;
    e3 = b - c;
    e = e2 - e1;
    out[1] = e * t3 + (e1 - e) * t2 + e3 * t + a;
    a = p0[2];
    b = p1[2];
    d = p3[2];
    c = p2[2];
    e1 = c - a;
    e2 = d - b;
    e3 = b - c;
    e = e2 - e1;
    out[2] = e * t3 + (e1 - e) * t2 + e3 * t + a;
    out[3] = 0.0f;
}
