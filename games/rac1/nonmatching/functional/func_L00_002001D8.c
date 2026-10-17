/* functional: high. Truncates x to an int (cvt.w.s, as the (int) cast does), stores that integer part as a float through ip and returns x minus it. Retail keeps the int round trip in the FPU (no GPR move); the value is the same, only the bytes differ (see NOTES.md). Corrected from the original: an out-of-range x saturates to 0x7FFFFFFF / 0x80000000 as cvt.w.s does on the EE, so the host's (int) cast never overflows. */
float func_L00_002001D8(float x, float *ip) {
    float t;
    if (x >= 2147483648.0f) {
        t = 2147483648.0f;
    } else if (x <= -2147483648.0f) {
        t = -2147483648.0f;
    } else {
        t = (float)(int)x;
    }
    *ip = t;
    return x - t;
}
