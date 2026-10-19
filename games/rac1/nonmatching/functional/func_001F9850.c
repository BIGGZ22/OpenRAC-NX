/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). scale_ticks: (int)((float)n * D_0015EE60 + 0.5f), truncated toward zero (cvt.w.s): ticks scaled by the frame-rate factor, rounded. */
extern float D_0015EE60 MACRO_ADDR;

int func_001F9850(int n) {
    return (int)((float)n * D_0015EE60 + 0.5f);
}
