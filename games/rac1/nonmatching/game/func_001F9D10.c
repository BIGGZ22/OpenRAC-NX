/* fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* FastVecDist: distance of two vectors, sqrt(dx*dx + dy*dy + dz*dz). */
extern float sqrtf(float);

float func_001F9D10(float *a, float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float dz = a[2] - b[2];
    float s = (dx * dx + dy * dy) + dz * dz;
    return sqrtf(s);
}
