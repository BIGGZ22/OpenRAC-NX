/* Square root of a float on the VU0 (vsqrt takes |x|, then vf0.x + Q
   adds 0.0 so that a zero result comes out +0). */
float func_001F9B50(float arg0) {
    return __builtin_sqrtf(__builtin_fabsf(arg0)) + 0.0f;
}
