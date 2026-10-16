/* FastAddRots(a, b): adds two angles in radians and wraps the sum into
   [-pi, pi): above pi, two subtractions of pi; below -pi (tested on the
   unwrapped sum), two additions of pi. */
float func_001FA748(float a, float b)
{
    const float pi = 3.14159274f;
    float s;
    int below;

    s = a + b;
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
