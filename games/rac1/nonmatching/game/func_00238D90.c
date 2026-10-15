/* moby_screen_rect: project two corner points a and b (vec4 each) through the
 * view matrix at D_00187180 - 0x40 to screen space, then write the rectangle as
 * x = a.x, y = a.y (the screen position of a) and w, h = the size from a to b. */
extern int D_0013E600[];
extern char D_0018CE00[];
extern int func_001FA898_r(float) __asm__("func_001FA898");

void func_00238D90(void *a, void *b, int *w, int *h, int *x, int *y) {
    float p[4];
    float q[4];
    float *pa = (float *)a;
    float *pb = (float *)b;
    float inv;
    float sx;
    float sy;
    int i;

    for (i = 0; i < 4; i++) {
        p[i] = pa[i];
        q[i] = pb[i];
    }
    func_001F9BF0(p, p, D_00187180);
    func_001F9BF0(q, q, D_00187180);
    func_001F9C30_a(p, p, 1024.0f);
    func_001F9C30_a(q, q, 1024.0f);
    q[3] = 1.0f;
    p[3] = 1.0f;
    func_001F9EE8(p, p, D_00187180 - 0x40);
    func_001F9EE8(q, q, D_00187180 - 0x40);

    inv = 1.0f / p[3];
    p[1] = p[1] * inv;
    p[0] = p[0] * inv;
    inv = 1.0f / q[3];
    q[0] = q[0] * inv;
    q[1] = q[1] * inv;

    sx = *(float *)(D_0018CE00 + 0x190);
    sy = *(float *)(D_0018CE00 + 0x194);
    p[0] = p[0] * sx;
    p[1] = p[1] * sy;
    q[0] = q[0] * sx;
    q[1] = q[1] * sy;

    *x = func_001FA898_r(p[0] * 0.25f + (float)D_0013E600[2]);
    *y = func_001FA898_r(p[1] * 0.25f + (float)D_0013E600[3]);
    *w = func_001FA898_r((q[0] - p[0]) * 0.25f);
    *h = func_001FA898_r((q[1] - p[1]) * 0.25f);
}
