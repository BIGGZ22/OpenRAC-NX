/* Blends two packed colours: each of the four bytes (low byte first) of a and b are mixed as
   a*(1-t) + b*t in VU0 floats, truncated to int and packed back into four bytes. */
int func_001FA8A8(int a, int b, float t) {
    unsigned int out = 0;
    int c;
    float ac;
    float bc;
    float p;
    float v;
    float w;
    int iv;

    w = 1.0f - t;
    for (c = 0; c < 4; c++) {
        ac = (float)((a >> (8 * c)) & 0xFF);
        bc = (float)((b >> (8 * c)) & 0xFF);
        p = ac * w;
        v = bc * t + p;
        iv = (int)v;
        out |= ((unsigned int)iv & 0xFF) << (8 * c);
    }
    return (int)out;
}
