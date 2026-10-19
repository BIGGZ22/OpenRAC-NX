/* fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* VecDistance2: distance of the x,y parts of two vectors, sqrt(dx*dx + dy*dy). */
extern float sqrtf(float);

float func_001F9D48(float *a, float *b) {
    float dx = a[0] - b[0];
    float dy = a[1] - b[1];
    float s = dx * dx + dy * dy;
    return sqrtf(s);
}
