extern int func_00282978(short *);
extern void func_00282CC0_17(char *, char *, char *);
extern void func_002B9180(void *);
extern unsigned char *D_001F2608[];
extern float D_001AA384 __attribute__((sda));

void func_002C1FC0(char *self)
{
    char *state = self + 32;
    if (func_00282978((short *)(self + 10)))
        *(float *)(state + 8) -= *(float *)(state + 24);
    func_00282CC0_17(self + 16, self + 16, state);
    self[8] = (unsigned char)self[8] + *(unsigned char *)(state + 20);
    if ((unsigned char)self[2] == D_001F2608[0][0])
        *(float *)(self + 12) += D_001AA384 * 3500.000244140625f;
    if (*(float *)(self + 24) < *(float *)(state + 16))
        func_002B9180(self);
}

extern float D_001AA380 __attribute__((sda));

void func_002C1B38(char *self)
{
    char *motion = self + 0x20;
    func_00282CC0_17(self + 0x10, self + 0x10, motion);
    if (*(float *)(self + 0x10) < 2.0f || *(float *)(self + 0x10) > 1021.0f ||
        *(float *)(self + 0x14) < 2.0f || *(float *)(self + 0x14) > 1021.0f ||
        *(float *)(self + 0x18) < 2.0f || *(float *)(self + 0x18) > 1021.0f) {
        func_002B9180(self);
        return;
    }
    *(float *)(self + 0x0C) += *(float *)(motion + 0x10);
    *(unsigned char *)(self + 8) += 1;
    if (*(float **)(motion + 0x14))
        *(float *)(self + 0x18) = **(float **)(motion + 0x14) + D_001AA380;
    if (--*(short *)(self + 0xA) == 0) {
        *(int *)(self + 4) -= 0x01000000;
        *(short *)(self + 0xA) = 4;
    }
    if ((*(unsigned int *)(self + 4) & 0xFF000000) == 0)
        func_002B9180(self);
}

extern int D_001AA39C __attribute__((sda));
extern int func_002A77E0(int);
extern int func_002A7820(int, int);
extern float func_002A7878(float, float);
extern void func_002BCB00(char *, char *, int, int, int, int, int, int, int, float);

void func_002C38E0(char *self)
{
    char *motion = self + 0x20;
    char *position;
    int ttl;
    if (func_00282978((short *)(self + 0xA)))
        *(float *)(motion + 8) -= *(float *)(motion + 0x14);
    position = self + 0x10;
    func_00282CC0_17(position, position, motion);
    *(unsigned char *)(self + 8) += 2;
    if (*(float *)(self + 0x18) < *(float *)(motion + 0x10)) {
        if (func_002A77E0(D_001AA39C - 1) == 0) {
            *(float *)(self + 0x18) = *(float *)(motion + 0x10);
            *(float *)(self + 0x20) *= func_002A7878(1.0f, 3.5f);
            *(float *)(motion + 4) *= func_002A7878(1.0f, 3.5f);
            *(float *)(motion + 8) *= func_002A7878(-1.0f, -1.5f);
            ttl = func_002A7820(60, 120);
            func_002BCB00(position, motion, *(int *)(self + 4), *(int *)(motion + 0x18),
                ttl, 0, *(unsigned char *)(self + 2), *(unsigned char *)(self + 3), 0,
                *(float *)(self + 0xC) * 0.5f);
        } else {
            *(float *)(self + 0x18) = *(float *)(motion + 0x10);
            *(float *)(self + 0x20) += func_002A7878(-5.0f, 5.0f) * 0.016666668f;
            *(float *)(motion + 4) += func_002A7878(-5.0f, 5.0f) * 0.016666668f;
            *(float *)(motion + 8) *= func_002A7878(-0.5f, -1.0f);
            ttl = func_002A7820(30, 60);
            func_002BCB00(position, motion, *(int *)(self + 4), *(int *)(motion + 0x18),
                ttl, 0, *(unsigned char *)(self + 2), *(unsigned char *)(self + 3), 0,
                *(float *)(self + 0xC) * 0.25f);
        }
        func_002B9180(self);
    }
}
