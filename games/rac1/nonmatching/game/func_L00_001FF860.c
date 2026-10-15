extern float D_L00_001C27A0[] NOT_SDA;
extern float D_L00_001C27C0[] NOT_SDA;

/* Angle of (b, a): atan2(b, a) in radians, from a rational reduction of min/max and an odd
   eight-term atan polynomial evaluated the way the VU0 does it (vector lanes x, y, z, w holding
   t, t^3, t^5, t^7 and so on); the octant pair from D_L00_001C27C0 is picked by the signs. */
float func_L00_001FF860(float a, float b) {
    float A;
    float B;
    float d;
    float lo;
    float hi;
    float t;
    float t2;
    float t4;
    float t8;
    float y3;
    float z5;
    float w5;
    float w7;
    float x9;
    float y11;
    float z13;
    float w15;
    float x1;
    float y1;
    float z1;
    float w1;
    float x2;
    float y2;
    float z2;
    float w2;
    float acc;
    float p;
    float pr;
    float t0;
    float t1;
    unsigned int sT;
    unsigned int s12;
    unsigned int s13;
    unsigned int k;

    A = (*(unsigned int *)&a >> 31) ? -a : a;
    B = (*(unsigned int *)&b >> 31) ? -b : b;
    d = A - B;
    sT = *(unsigned int *)&d >> 31;
    if (sT) {
        lo = A;
        hi = B;
    } else {
        lo = B;
        hi = A;
    }
    if (hi <= 0.0f) {
        return 0.0f;
    }
    t = (lo - hi) / (lo + hi);

    /* vector part: vf1 = (t, t, t, t) */
    x1 = t;
    y1 = t;
    z1 = t;
    w1 = t;
    t2 = t * t;
    t4 = t2 * t2;
    t8 = t4 * t4;
    y1 = y1 * t2;
    z1 = z1 * t4;
    w1 = w1 * t4;
    w1 = w1 * t2;
    x2 = x1 * t8;
    y2 = y1 * t8;
    z2 = z1 * t8;
    w2 = w1 * t8;
    x1 = x1 * D_L00_001C27A0[0];
    y1 = y1 * D_L00_001C27A0[1];
    z1 = z1 * D_L00_001C27A0[2];
    w1 = w1 * D_L00_001C27A0[3];
    x2 = x2 * D_L00_001C27A0[4];
    y2 = y2 * D_L00_001C27A0[5];
    z2 = z2 * D_L00_001C27A0[6];
    w2 = w2 * D_L00_001C27A0[7];

    /* accumulate: ACC.x = x + y, then += z, w, x2, y2, z2; result = ACC + w2 */
    acc = x1 + y1;
    acc = acc + z1;
    acc = acc + w1;
    acc = acc + x2;
    acc = acc + y2;
    acc = acc + z2;
    p = acc + w2;

    pr = 0.78539818525f + p;
    s12 = *(unsigned int *)&a >> 31;
    s13 = *(unsigned int *)&b >> 31;
    k = sT * 2 + s13 * 4 + s12 * 8;
    t0 = D_L00_001C27C0[k];
    t1 = D_L00_001C27C0[k + 1];
    pr = pr * t0;
    return pr + t1;
}
