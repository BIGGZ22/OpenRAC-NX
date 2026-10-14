/* functional: medium. Range test on x: for a < 0xE0 returns 1 if x >= 47.75f else 0; for a >= 0xE0 returns 1 if x < 29.0f else 0. */
int func_L00_0024A140(int a, float x) {
    if (a < 0xE0) {
        return 47.75f <= x;
    }
    return x < 29.0f;
}
