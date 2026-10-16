/* FastArcSin: |x| through c0 + c1*a + c2*a^2 + c3*a^3 (the constants at gp-0x6560, 0x1607A0, read from
   the game's memory), times sqrt(1 - |x|) (VU0 vsqrt, which takes the magnitude), subtracted from pi/2
   and given the sign of x. From the earlier worker's C (build-sn/fill/func_001F9FC0), which used
   literal constants and sqrtf. */
extern float openrac_sqrt(float);
extern float D_001607A0_f[] __asm__("D_001607A0");
float func_001F9FC0(float x) {
    float sign = 1.0f;
    float ax;
    float t;
    float sq;
    float a2;
    float a3;
    float acc;
    float poly;
    union { unsigned int u; float f; } h;

    if (x < 0.0f) {
        sign = -1.0f;
        ax = -x;
    } else {
        ax = x;
    }
    t = 1.0f - ax;
    sq = openrac_sqrt(t < 0.0f ? -t : t);
    a2 = ax * ax;
    a3 = ax * a2;
    acc = D_001607A0_f[0] + 0.0f;
    acc = acc + ax * D_001607A0_f[1];
    acc = acc + a2 * D_001607A0_f[2];
    poly = acc + a3 * D_001607A0_f[3];
    h.u = 0x3FC90FDB;
    return sign * (h.f - poly * sq);
}
