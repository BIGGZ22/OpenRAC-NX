/* functional: high. fcheck: unchecked (never called in fcheck's capture run of the title, menu and level 0; OpenRAC tools/fcheck_gate.py, 2026-10-18). Written from the assembly (no C existed). FastSubRots(a, b): a - b in radians, wrapped into [-pi, pi) with the same two-step rule as FastAddRots; the wrap test is taken on the unwrapped difference. Nothing differs from retail that I could find. */
float func_001FA790(float a, float b)
{
    const float pi = 3.14159274f;
    float s;
    int below;

    s = a - b;
    below = s < -pi;
    if (s >= pi) {
        s = s - pi;
        s = s - pi;
    }
    if (below) {
        s = s + pi;
        s = s + pi;
    }
    return s;
}
