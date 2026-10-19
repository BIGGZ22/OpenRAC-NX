/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Squared length of the vector at a0 (x*x + y*y + z*z) returned as a float, which the VU0 code computes in its accumulator; the summation order and rounding of the VU may differ from plain C. */
float func_L00_001FF3E0(float *v) {
    return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
}
