/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Stores the vector (0, 0, 1, 0) to dst: VU0 vmr32 rotates vf0 = (0, 0, 0, 1) by one element; relies on that rotation direction. */

void func_001F9BC8(float *dst) {
    dst[0] = 0.0f;
    dst[1] = 0.0f;
    dst[2] = 1.0f;
    dst[3] = 0.0f;
}
