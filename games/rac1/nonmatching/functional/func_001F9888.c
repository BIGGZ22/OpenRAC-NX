/* functional: high. As func_001F9850 with the factor at D_0015EE64: (int)((float)n * D_0015EE64 + 0.5f), truncated toward zero. */
extern float D_0015EE64 MACRO_ADDR;

int func_001F9888(int n) {
    return (int)((float)n * D_0015EE64 + 0.5f);
}
