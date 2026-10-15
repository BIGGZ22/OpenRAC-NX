/* Wraps an angle in radians into [-pi, pi): subtracts or adds 2*pi
   (two single-precision steps of pi each) until the value is in range. */
float func_001FA7D8(float x)
{
    const float pi = 3.14159274f;

    if (!(x < pi)) {
        do {
            x = x - pi;
            x = x - pi;
        } while (!(x < pi));
    }
    if (x < -pi) {
        do {
            x = x + pi;
            x = x + pi;
        } while (x < -pi);
    }
    return x;
}
