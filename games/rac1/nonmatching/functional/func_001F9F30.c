/* functional: high. Corrected from nonmatching/game/func_001F9F30.c: ppach/ppacb keep the LOW halfword/byte of each word, so the result is the low byte of each saturated int (x lowest), not the top byte (>> 24). */
static int func_001F9F30_ftoi(float f) {
    if (f >= 2147483648.0f) return 0x7FFFFFFF;
    if (f <= -2147483648.0f) return (int)0x80000000;
    return (int)f;
}

int func_001F9F30(float *v) {
    int x = func_001F9F30_ftoi(v[0]);
    int y = func_001F9F30_ftoi(v[1]);
    int z = func_001F9F30_ftoi(v[2]);
    int w = func_001F9F30_ftoi(v[3]);
    return (x & 0xFF) | ((y & 0xFF) << 8) | ((z & 0xFF) << 16) | ((w & 0xFF) << 24);
}
