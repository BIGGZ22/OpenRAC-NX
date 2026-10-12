/* functional: high. Truncates x to an int (cvt.w.s, as the (int) cast does), stores that integer part as a float through ip and returns x minus it. Retail keeps the int round trip in the FPU (no GPR move); the value is the same, only the bytes differ (see NOTES.md). */
float func_L00_002001D8(float x, float *ip) {
    float t = (float)(int)x;
    *ip = t;
    return x - t;
}
