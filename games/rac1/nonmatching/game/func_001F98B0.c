/* fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* Scales its float argument by the small-data float at gp-0x7E98 (D_0015EE68). */
extern float D_0015EE68 MACRO_ADDR;

float func_001F98B0(float x)
{
    return D_0015EE68 * x;
}
