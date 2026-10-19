/* functional: medium. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). VU0 microcode written as plain floats: the sphere overlap test. Retail takes the difference of the two xyz, sums the squares, subtracts the square of the sum of the w fields (the radii), and returns 1 when that is negative (the spheres overlap), 0 otherwise. Which lane holds the result (y, read as the high word of qmfc2) was worked out from the instruction sequence and the sign-bit branch; checked against lights.c's callers, which pass a sphere whose w is its radius. */
int func_001F9D78(void *pa, void *pb) {
    float *a = (float *)pa;
    float *b = (float *)pb;
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    float ws = a[3] + b[3];
    float r = (dx * dx + dy * dy + dz * dz) - ws * ws;

    return r < 0.0f;
}
