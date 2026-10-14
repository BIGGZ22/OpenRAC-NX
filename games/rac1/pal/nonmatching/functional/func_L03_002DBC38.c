/* functional: high. Only registers, scheduling, branch shapes and the frame layout differ from retail (its duplicated delay-slot loads are single reads here), so the reads, writes, calls, arguments and their order are retail's. */
/* Level 3 Helga (class 890) update: state 0 waits for the level flag or the
 * stage gate, then starts her animation (state 1); state 1 waits for it and
 * goes to state 2; state 2 handles the pause help and the end of the talk.
 * The tail turns her head toward a look target (the hero, or a random point
 * picked on a timer) and runs her two head/eye animators. */
typedef unsigned int u128_DBC38 __attribute__((mode(TI), aligned(16)));

extern void func_L03_002DBAD8_DBC38(unsigned char *) __asm__("func_L03_002DBAD8");
extern int func_L00_002676E8_DBC38(unsigned char *, unsigned char *) __asm__("func_L00_002676E8");
extern int func_L00_00267290_DBC38(unsigned char *, unsigned char *) __asm__("func_L00_00267290");
extern void func_L01_00279398_DBC38(float, unsigned char *) __asm__("func_L01_00279398");
extern void func_L00_002512D8_DBC38(int) __asm__("func_L00_002512D8");
extern void func_L00_00286128_DBC38(unsigned char *, unsigned char *) __asm__("func_L00_00286128");
extern void func_L00_002618D8_DBC38(int, int) __asm__("func_L00_002618D8");
extern void func_L00_00264DB8_DBC38(int, int) __asm__("func_L00_00264DB8");
extern void func_0020D678_DBC38(unsigned char *) __asm__("func_0020D678");
extern int func_0020BFC8_DBC38(int, int) __asm__("func_0020BFC8");
extern float func_001F9D48_DBC38(void *, void *) __asm__("func_001F9D48");
extern float func_L00_001FF860_DBC38(float, float) __asm__("func_L00_001FF860");
extern float func_001FA850_DBC38(float, float) __asm__("func_001FA850");
extern float func_001F9CB8_DBC38(void *) __asm__("func_001F9CB8");
extern int func_001F9850_DBC38(int) __asm__("func_001F9850");
extern int func_001F9908_DBC38(int *) __asm__("func_001F9908");
extern float func_002140F8_DBC38(float, float) __asm__("func_002140F8");
extern float func_001F9878_DBC38(float) __asm__("func_001F9878");
extern int func_001FA898_DBC38(float) __asm__("func_001FA898");
extern float func_001FA748_DBC38(float, float) __asm__("func_001FA748");
extern void func_00215C00_DBC38(void *, float, float, float) __asm__("func_00215C00");
extern void func_001F9BD8_DBC38(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0_DBC38(void *, void *, void *) __asm__("func_001F9BF0");
extern float func_001FA790_DBC38(float, float) __asm__("func_001FA790");
extern float func_001F9CE8_DBC38(void *) __asm__("func_001F9CE8");
extern void func_L00_00263950_DBC38(unsigned char *, unsigned char *, int, float, float) __asm__("func_L00_00263950");
extern void func_L02_0025D750_DBC38(unsigned char *) __asm__("func_L02_0025D750");
extern void func_L01_0029C2A0_DBC38(unsigned char *, int, unsigned char *, int, void *) __asm__("func_L01_0029C2A0");

extern unsigned char D_0013D5CA_DBC38[] __asm__("D_0013D5CA");
extern unsigned char D_0014171B_DBC38[] __asm__("D_0014171B");
extern unsigned char D_0013E633_DBC38[] __asm__("D_0013E633");
extern int D_0015EE84_DBC38 __asm__("D_0015EE84");
extern float D_0015EE64_DBC38 __asm__("D_0015EE64");
extern unsigned char D_0015EEB0_DBC38 __asm__("D_0015EEB0");
extern int D_L03_0015F6A8_DBC38 __asm__("D_L03_0015F6A8");
extern unsigned char *D_L03_00160058_DBC38 __asm__("D_L03_00160058");
extern unsigned char *D_L03_0016016C_DBC38 __asm__("D_L03_0016016C");
extern unsigned char D_L03_001BAA30_DBC38[] __asm__("D_L03_001BAA30");
extern unsigned char D_L03_00161C38_DBC38[] __asm__("D_L03_00161C38");
extern unsigned char D_L03_001BA9E0_DBC38[] __asm__("D_L03_001BA9E0");
extern unsigned char D_L03_001BB640_DBC38[] __asm__("D_L03_001BB640");
extern unsigned char D_L03_0015FD08_DBC38[] __asm__("D_L03_0015FD08");

void func_L03_002DBC38(unsigned char *m) {
    unsigned char *d = *(unsigned char **)(m + 0x78);
    unsigned char *p16 = d + 0x80;
    unsigned char *P = D_0013E633_DBC38 + 0xE9D; /* 0x13F4D0 */
    unsigned char *stage = D_0014171B_DBC38 + 0xAA35; /* 0x14C150 */
    int st;
    int looked = 0;
    float f22, f23;
    u128_DBC38 loc;
    float v2[4] __attribute__((aligned(16)));
    float v3[4] __attribute__((aligned(16)));

    func_L03_002DBAD8_DBC38(m);
    st = m[0x20];
    if (st == 0) {
        if (D_0013D5CA_DBC38[10] != 0) {
            {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x8C) = 1;
                    func_L01_0029C2A0_DBC38(q + 0x8C, 4, m, 2, D_L03_001BAA30_DBC38);
                }
            }
        }
            func_0020D678_DBC38(m);
            return;
        }
        if (stage[m[0xB0] + (D_0015EE84_DBC38 << 4)] == 0xFF) {
            {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x8C) = 1;
                    func_L01_0029C2A0_DBC38(q + 0x8C, 4, m, 2, D_L03_001BAA30_DBC38);
                }
            }
        }
        }
        {
            unsigned char *G = *(unsigned char **)(D_0013E633_DBC38 + 0x2E9D);
            m[0x20] = 1;
            *(long *)(m + 0x38) = *(long *)(G + 0x38);
            *(unsigned char **)(p16 + 0x20) = D_L03_00161C38_DBC38;
            *(int *)(d + 0xC8) = 5;
            func_L00_002676E8_DBC38(m, p16);
            func_L02_0025D750_DBC38(m);
        }
    } else if (st == 1) {
        if (func_L00_00267290_DBC38(m, p16) != 0) {
            func_L01_00279398_DBC38(2.3f, m);
            m[0x20] = 2;
            {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x8C) = 1;
                    func_L01_0029C2A0_DBC38(q + 0x8C, 4, m, 2, D_L03_001BAA30_DBC38);
                }
            }
        }
        }
    } else if (st == 2) {
        if (D_L03_0015F6A8_DBC38 == 2) {
            if (*(short *)(d + 0xB6) == 2) {
                {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x7C) = 0;
                    *(int *)(q + 0x8C) = 0;
                    *(int *)(q + 0xA0) = *(int *)(d + 0xD8);
                }
            }
        }
            }
        } else {
            if (*(short *)(d + 0x84) != 2) {
                func_L00_002512D8_DBC38(m[0xB0]);
                {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x8C) = 1;
                    func_L01_0029C2A0_DBC38(q + 0x8C, 4, m, 2, D_L03_001BAA30_DBC38);
                }
            }
        }
                {
                    int w3 = *(int *)(d + 0xCC);
                    if (w3 != -1) {
                        unsigned char *base = D_L03_0016016C_DBC38 + (w3 << 7);
                        func_L00_00286128_DBC38(base + 0x30, base + 0x70);
                    }
                }
            }
            m[0x20] = 1;
            /* retail reads +0x84 again here */
            if (*(short *)(d + 0x84) == 2) {
                func_L00_002618D8_DBC38(12, 1);
                func_L00_00264DB8_DBC38(0xBC8, -1);
                if (*(int *)(d + 0xD0) != -1) {
                    unsigned char *E = D_L03_00160058_DBC38 + (*(int *)(d + 0xD0) << 8);
                    unsigned char b6;
                    D_L03_001BA9E0_DBC38[*(short *)(E + 0xB2) + 0x454] = E[0xB0] + 2;
                    E = D_L03_00160058_DBC38 + (*(int *)(d + 0xD0) << 8);
                    b6 = E[0xB0];
                    if (b6 == 0xFF
                        || (D_L03_0015FD08_DBC38[b6] != 0xFF
                            && stage[b6 + (D_0015EE84_DBC38 << 4)] == 0xFF)) {
                        D_L03_001BB640_DBC38[*(short *)(E + 0xB2) + 0x454] = b6 + 2;
                    }
                    func_0020D678_DBC38(D_L03_00160058_DBC38 + (*(int *)(d + 0xD0) << 8));
                }
                {
            int w = *(int *)(d + 0xD4);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DBC38 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    unsigned char *q = *(unsigned char **)(e + 0x78);
                    *(int *)(q + 0x7C) = 0;
                    *(int *)(q + 0x8C) = 0;
                    *(int *)(q + 0xA0) = *(int *)(d + 0xD8);
                }
            }
        }
                func_0020BFC8_DBC38(0, -1);
                func_0020D678_DBC38(m);
                return;
            }
        }
    }

    /* tail: the look target, only while m[0x53] is zero */
    f22 = 0.02f;
    f23 = 0.3f;
    if (m[0x53] == 0) {
        int near = 0;
        looked = 1;
        if (func_001F9D48_DBC38(m + 0x10, P) < 8.0f) {
            float a = func_L00_001FF860_DBC38(*(float *)(P - 0x80 + 0xD0) - *(float *)(m + 0x10),
                                              *(float *)(P - 0x80 + 0xD4) - *(float *)(m + 0x14));
            if (func_001FA850_DBC38(*(float *)(m + 0x48), a) < 1.5707964f) {
                near = 1;
                if (0.01f < func_001F9CB8_DBC38(P + 0x80)) {
                    *(int *)(d + 0x1F0) = func_001F9850_DBC38(0x78);
                } else {
                    func_001F9908_DBC38((int *)(d + 0x1F0));
                }
            }
        }
        if (!near && *(int *)(d + 0x1F0) != 0) {
            *(int *)(d + 0x1F0) = 0;
            *(u128_DBC38 *)(d + 0x1E0) = *(u128_DBC38 *)(P + 0x50);
        }
        if (func_001F9908_DBC38((int *)(d + 0x1F4)) != 0) {
            const float k = 0.0174532925f;
            float yaw, pitch;
            *(int *)(d + 0x1F4) = func_001FA898_DBC38(func_001F9878_DBC38(func_002140F8_DBC38(180.0f, 300.0f)));
            yaw = func_001FA748_DBC38(*(float *)(m + 0x48), func_002140F8_DBC38(-90.0f, 90.0f) * k);
            pitch = func_002140F8_DBC38(0.0f, 30.0f) * k;
            func_00215C00_DBC38(d + 0x1E0, 6.0f, yaw, pitch);
            func_001F9BD8_DBC38(d + 0x1E0, d + 0x1E0, m + 0x10);
        }
        if (*(int *)(d + 0x1F0) != 0) {
            loc = *(u128_DBC38 *)(P + 0x50);
            f22 = 0.04f;
            f23 = 0.3f;
        } else {
            loc = *(u128_DBC38 *)(d + 0x1E0);
        }
    }
    if (looked) {
        float yaw, pitch;
        *(u128_DBC38 *)v2 = *(u128_DBC38 *)(m + 0x10);
        v2[2] = v2[2] + 1.0f;
        func_001F9BF0_DBC38(v3, &loc, v2);
        yaw = func_001FA790_DBC38(func_L00_001FF860_DBC38(v3[0], v3[1]), *(float *)(m + 0x48));
        pitch = -func_L00_001FF860_DBC38(func_001F9CE8_DBC38(v3), v3[2]);
        if (1.5707964f < yaw) {
            yaw = 1.5707964f;
        } else if (yaw < -1.5707964f) {
            yaw = -1.5707964f;
        }
        if (0.5235988f < pitch) {
            pitch = 0.5235988f;
        } else if (pitch < -0.5235988f) {
            pitch = -0.5235988f;
        }
        *(float *)(d + 0x144) = pitch;
        *(float *)(d + 0x148) = yaw * 0.5f;
    }
    if (D_0015EEB0_DBC38 != 0) {
        *(float *)(d + 0x150) = 2.75f;
    }
    func_L00_00263950_DBC38(m, d + 0xE0, 0, f22 * D_0015EE64_DBC38, f23 * D_0015EE64_DBC38);
    func_L00_00263950_DBC38(m, d + 0x160, 1, f22 * D_0015EE64_DBC38, f23 * D_0015EE64_DBC38);
}
