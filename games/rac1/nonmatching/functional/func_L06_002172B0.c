/* functional: high. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Stores the architectural VU0 constant vf0 = (0, 0, 0, 1) as four floats at a0; vf0 is hardwired on the VU0, so the values are exact. Alignment of the 16-byte store is not modelled. */
void func_L06_002172B0(float *p)
{
    p[0] = 0.0f;
    p[1] = 0.0f;
    p[2] = 0.0f;
    p[3] = 1.0f;
}
