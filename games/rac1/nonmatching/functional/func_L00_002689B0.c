/* functional: medium. fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). Written from the assembly (hand-written VU0 particle step, no C existed). Plain float arithmetic for the VU0 ops; the divide by a zero total life (VU gives the largest float) is clamped to FLT_MAX, and float to byte conversion saturates as vftoi0 does. */
extern void func_L00_002688A8(void *);

static int ftoi0_002689B0(float f)
{
    if (f >= 2147483520.0f) {
        return 0x7FFFFFFF;
    }
    if (f <= -2147483648.0f) {
        return (int)0x80000000;
    }
    return (int)f;
}

/* Particle ageing step. p+0xA is the remaining life (short), p+0x38 the total life, p+8 a tick
   counter (byte). Life is decremented; at 0 or below the particle is handed to func_L00_002688A8
   (the tail call func_L00_00268A88 makes). Otherwise: position (p+0x10, xyz) += velocity
   (p+0x20, xyz), velocity z += velocity w; the colour at p+4 (4 bytes) is the blend
   c1*(1-q) + c0*q with q = life/total, c0 the bytes at p+0x30 and c1 the bytes at p+0x34,
   and the alpha bytes are first scaled by min(8, life)/8 (the fade-out over the last 8
   ticks); p+0xC (float) += p+0x3C. */
void func_L00_002689B0(char *p)
{
    int life = *(short *)(p + 0xA) - 1;
    int total;
    unsigned int u0, u1;
    float c0[4], c1[4], q, fade, ow;
    float *pos, *vel;
    unsigned char out[4];
    int i;

    if (life <= 0) {
        func_L00_002688A8(p);
        return;
    }
    total = *(short *)(p + 0x38);
    u0 = *(unsigned int *)(p + 0x30);
    u1 = *(unsigned int *)(p + 0x34);
    for (i = 0; i < 4; i++) {
        c0[i] = (float)((u0 >> (8 * i)) & 0xFF);
        c1[i] = (float)((u1 >> (8 * i)) & 0xFF);
    }
    *(short *)(p + 0xA) = life;
    if (total == 0) {
        q = 3.4028234e38f;
    } else {
        q = (float)life / (float)total;
    }
    fade = (float)(life < 8 ? life : 8) / 8.0f;
    *(unsigned char *)(p + 8) += 1;

    pos = (float *)(p + 0x10);
    vel = (float *)(p + 0x20);
    pos[0] += vel[0];
    pos[1] += vel[1];
    pos[2] += vel[2];
    vel[2] += vel[3];

    c0[3] *= fade;
    c1[3] *= fade;
    ow = 1.0f - q;
    for (i = 0; i < 4; i++) {
        out[i] = (unsigned char)ftoi0_002689B0(c1[i] * ow + c0[i] * q);
    }
    *(float *)(p + 0xC) = *(float *)(p + 0xC) + *(float *)(p + 0x3C);
    *(unsigned int *)(p + 4) = out[0] | (out[1] << 8) | (out[2] << 16) | ((unsigned int)out[3] << 24);
}
