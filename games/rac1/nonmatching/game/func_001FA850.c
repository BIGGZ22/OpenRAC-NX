/*
 * The unsigned angle between two angles a and b (radians): |a - b|, or 2*pi - |a - b|
 * when that is the shorter way round (|a - b| >= pi).
 */
float func_001FA850(float a, float b)
{
    float d;

    d = __builtin_fabsf(a - b);
    if (!(d < 3.14159265f)) {
        d = (3.14159265f + 3.14159265f) - d;
    }
    return d;
}
