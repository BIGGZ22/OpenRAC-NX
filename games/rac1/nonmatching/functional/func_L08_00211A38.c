/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Written from the assembly, not compiled. Same reads (*p, D_0015EE7C), same stores (0 or x - threshold, as an int or float), same return codes (-1 when the bit pattern of the original value is zero or negative, -2 when positive, 0 on the store path); the clamp test is an ordered compare, which is false for NaN exactly as c.le.s is. */
extern float D_0015EE7C MACRO_ADDR;

int func_L08_00211A38(float *p) {
    float x = p[0];
    float lim = D_0015EE7C;
    if (x <= lim) {
        int bits = *(int *)&x;
        *(int *)p = 0;
        return ((bits <= 0) ? 1 : 0) - 2;
    }
    p[0] = x - lim;
    return 0;
}
