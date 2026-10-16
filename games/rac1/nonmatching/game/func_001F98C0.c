extern float D_0015EE68 MACRO_ADDR;

/* func_001F98C0(x): returns (int)(0.5f + (float)x * D_0015EE68), truncated
   toward zero, the rounding of EE cvt.w.s. The 0.5 is the accumulator that
   adda.s loads from two copies of 0.25f before the madd. */
int func_001F98C0(int a) {
    float v = (float)a * D_0015EE68 + 0.5f;

    return (int)v;
}
