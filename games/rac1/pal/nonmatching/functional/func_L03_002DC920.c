/* functional: high. Only registers, scheduling, branch shapes and the frame layout differ from retail, so the reads, writes, calls, arguments and their order are retail's (the unguarded last func_L00_00217718 call keeps retail's index even when it is -1). */
/* Level 3 (Kerwan) class 909 NPC update: state 0 faces it (turning half a
 * turn unless the global at 0x13D804 is set), clears a linked moby's flag
 * when the stage slot is empty and starts its animation (state 1); state 1
 * waits for the level flag / stage gate and the animation, then arms its
 * linked mobys and goes to state 2; state 2 handles the talk (when the level
 * mode is not 2) and, at its end, plays the sound and fires the linked
 * triggers. The tail turns its head toward a look target (the hero, or a
 * random point picked on a timer) and runs its two head/eye animators. */
typedef unsigned int u128_DC920 __attribute__((mode(TI), aligned(16)));

extern void func_L03_002DC848_DC920(unsigned char *) __asm__("func_L03_002DC848");
extern int func_L00_002676E8_DC920(unsigned char *, unsigned char *) __asm__("func_L00_002676E8");
extern int func_L00_00267290_DC920(unsigned char *, unsigned char *) __asm__("func_L00_00267290");
extern void func_L01_00279398_DC920(float, unsigned char *) __asm__("func_L01_00279398");
extern void func_L00_002512D8_DC920(int) __asm__("func_L00_002512D8");
extern void func_L00_00286128_DC920(unsigned char *, unsigned char *) __asm__("func_L00_00286128");
extern void func_L00_002618D8_DC920(int, int) __asm__("func_L00_002618D8");
extern void func_L00_00264DB8_DC920(int, int) __asm__("func_L00_00264DB8");
extern int func_0020BFC8_DC920(int, int) __asm__("func_0020BFC8");
extern void func_L00_00217718_DC920(void *, void *, int, int) __asm__("func_L00_00217718");
extern float func_001F9D48_DC920(void *, void *) __asm__("func_001F9D48");
extern float func_L00_001FF860_DC920(float, float) __asm__("func_L00_001FF860");
extern float func_001FA850_DC920(float, float) __asm__("func_001FA850");
extern float func_001F9CB8_DC920(void *) __asm__("func_001F9CB8");
extern int func_001F9850_DC920(int) __asm__("func_001F9850");
extern int func_001F9908_DC920(int *) __asm__("func_001F9908");
extern float func_002140F8_DC920(float, float) __asm__("func_002140F8");
extern float func_001F9878_DC920(float) __asm__("func_001F9878");
extern int func_001FA898_DC920(float) __asm__("func_001FA898");
extern float func_001FA748_DC920(float, float) __asm__("func_001FA748");
extern void func_00215C00_DC920(void *, float, float, float) __asm__("func_00215C00");
extern void func_001F9BD8_DC920(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001F9BF0_DC920(void *, void *, void *) __asm__("func_001F9BF0");
extern float func_001FA790_DC920(float, float) __asm__("func_001FA790");
extern float func_001F9CE8_DC920(void *) __asm__("func_001F9CE8");
extern void func_L00_00263950_DC920(unsigned char *, unsigned char *, int, float, float) __asm__("func_L00_00263950");
extern void func_L02_0025D750_DC920(unsigned char *) __asm__("func_L02_0025D750");

extern unsigned char D_0013D5CA_DC920[] __asm__("D_0013D5CA");
extern unsigned char D_0013D50F_DC920[] __asm__("D_0013D50F");
extern unsigned char D_0013D605_DC920[] __asm__("D_0013D605");
extern unsigned char D_0014171B_DC920[] __asm__("D_0014171B");
extern unsigned char D_0013E633_DC920[] __asm__("D_0013E633");
extern int D_0015EE84_DC920 __asm__("D_0015EE84");
extern float D_0015EE64_DC920 __asm__("D_0015EE64");
extern unsigned char D_0015EEB0_DC920 __asm__("D_0015EEB0");
extern int D_L03_0015F6A8_DC920 __asm__("D_L03_0015F6A8");
extern unsigned char *D_L03_00160058_DC920 __asm__("D_L03_00160058");
extern unsigned char *D_L03_0016016C_DC920 __asm__("D_L03_0016016C");
extern unsigned char D_L03_001BA9E0_DC920[] __asm__("D_L03_001BA9E0");
extern unsigned char D_L03_001BB640_DC920[] __asm__("D_L03_001BB640");
extern unsigned char D_L03_0015FD08_DC920[] __asm__("D_L03_0015FD08");

void func_L03_002DC920(unsigned char *m) {
    unsigned char *d = *(unsigned char **)(m + 0x78);
    unsigned char *P = D_0013E633_DC920 + 0xE9D; /* 0x13F4D0 */
    unsigned char *stage = D_0014171B_DC920 + 0xAA35; /* 0x14C150 */
    unsigned char *flags = D_0013D50F_DC920 + 0xB9; /* 0x13D5C8 */
    int st;
    int looked = 0;
    float f22, f23;
    u128_DC920 loc;
    float v2[4] __attribute__((aligned(16)));
    float v3[4] __attribute__((aligned(16)));

    func_L03_002DC848_DC920(m);
    st = m[0x20];
    if (st == 0) {
        if (*(int *)(D_0013D605_DC920 + 0x1FF) != 0) {
            *(int *)(d + 0x140) = 1;
        } else {
            *(float *)(m + 0x48) = func_001FA748_DC920(*(float *)(m + 0x48), 3.1415927f);
            *(int *)(d + 0x140) = 0;
        }
        if (stage[m[0xB0] + (D_0015EE84_DC920 << 4)] == 0xFF) {
            {
            int w = *(int *)(d + 0x148);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DC920 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    *(int *)(*(unsigned char **)(e + 0x78) + 0x8C) = 1;
                }
            }
        }
        }
        {
            unsigned char *G = *(unsigned char **)(D_0013E633_DC920 + 0x2E9D);
            m[0x20] = 1;
            *(long *)(m + 0x38) = *(long *)(G + 0x38);
            func_L00_002676E8_DC920(m, d);
            func_L02_0025D750_DC920(m);
        }
        if (D_0013D5CA_DC920[0] != 0) {
            *(short *)(d + 0x36) = -1;
        }
    } else if (st == 1) {
        int go = 1;
        if (flags[2] != 0) {
            {
            int w = *(int *)(d + 0x144);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DC920 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3E5 && e[0x20] == 0) {
                    unsigned char *E;
                    unsigned char b6;
                    e[0x20] = 1;
                    E = D_L03_00160058_DC920 + (*(int *)(d + 0x144) << 8);
                    D_L03_001BA9E0_DC920[*(short *)(E + 0xB2) + 0x454] = E[0xB0] + 2;
                    E = D_L03_00160058_DC920 + (*(int *)(d + 0x144) << 8);
                    b6 = E[0xB0];
                    if (b6 == 0xFF
                        || (D_L03_0015FD08_DC920[b6] != 0xFF
                            && stage[b6 + (D_0015EE84_DC920 << 4)] == 0xFF)) {
                        D_L03_001BB640_DC920[*(short *)(E + 0xB2) + 0x454] = b6 + 2;
                    }
                }
            }
        }
            if (flags[2] != 0 && stage[m[0xB0] + (D_0015EE84_DC920 << 4)] == 0xFF) {
                go = 0;
            }
        }
        if (go && func_L00_00267290_DC920(m, d) != 0) {
            if (*(int *)(d + 0x140) == 0) {
                *(float *)(m + 0x48) = func_001FA748_DC920(*(float *)(m + 0x48), 3.1415927f);
                *(int *)(d + 0x140) = 1;
            }
            func_L01_00279398_DC920(2.7f, m);
            *(int *)(m + 0x58) = 0;
            m[0x20] = 2;
        }
    } else if (st == 2) {
        if (D_L03_0015F6A8_DC920 != 2) {
            *(float *)(m + 0x58) = 1.0f;
            if (*(short *)(d + 4) != 2) {
                int w3;
                func_L00_002512D8_DC920(m[0xB0]);
                {
            int w = *(int *)(d + 0x148);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DC920 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3F4) {
                    *(int *)(*(unsigned char **)(e + 0x78) + 0x8C) = 1;
                }
            }
        }
                w3 = *(int *)(d + 0x14C);
                if (w3 != -1) {
                    unsigned char *base = D_L03_0016016C_DC920 + (w3 << 7);
                    func_L00_00286128_DC920(base + 0x30, base + 0x70);
                }
            }
            m[0x20] = 1;
            if (*(short *)(d + 4) == 2) {
                int w3;
                unsigned char *base;
                func_L00_002618D8_DC920(2, 1);
                func_L00_00264DB8_DC920(0xBC7, -1);
                {
            int w = *(int *)(d + 0x144);
            if (w != -1) {
                unsigned char *e = D_L03_00160058_DC920 + (w << 8);
                if (*(short *)(e + 0xA6) == 0x3E5 && e[0x20] == 0) {
                    unsigned char *E;
                    unsigned char b6;
                    e[0x20] = 1;
                    E = D_L03_00160058_DC920 + (*(int *)(d + 0x144) << 8);
                    D_L03_001BA9E0_DC920[*(short *)(E + 0xB2) + 0x454] = E[0xB0] + 2;
                    E = D_L03_00160058_DC920 + (*(int *)(d + 0x144) << 8);
                    b6 = E[0xB0];
                    if (b6 == 0xFF
                        || (D_L03_0015FD08_DC920[b6] != 0xFF
                            && stage[b6 + (D_0015EE84_DC920 << 4)] == 0xFF)) {
                        D_L03_001BB640_DC920[*(short *)(E + 0xB2) + 0x454] = b6 + 2;
                    }
                }
            }
        }
                w3 = *(int *)(d + 0x14C);
                if (w3 != -1) {
                    base = D_L03_0016016C_DC920 + (w3 << 7);
                    func_L00_00286128_DC920(base + 0x30, base + 0x70);
                }
                func_0020BFC8_DC920(0, -1);
                /* retail does not check for -1 here */
                base = D_L03_0016016C_DC920 + (*(int *)(d + 0x14C) << 7);
                func_L00_00217718_DC920(base + 0x30, base + 0x70, 0, 1);
            }
        }
    }

    /* tail: the look target, only while m[0x53] is zero */
    f22 = 0.02f;
    f23 = 0.3f;
    if (m[0x53] == 0) {
        int near = 0;
        looked = 1;
        if (func_001F9D48_DC920(m + 0x10, P) < 8.0f) {
            float a = func_L00_001FF860_DC920(*(float *)(P - 0x80 + 0xD0) - *(float *)(m + 0x10),
                                              *(float *)(P - 0x80 + 0xD4) - *(float *)(m + 0x14));
            if (func_001FA850_DC920(*(float *)(m + 0x48), a) < 1.5707964f) {
                near = 1;
                if (0.01f < func_001F9CB8_DC920(P + 0x80)) {
                    *(int *)(d + 0x164) = func_001F9850_DC920(0x78);
                } else {
                    func_001F9908_DC920((int *)(d + 0x164));
                }
            }
        }
        if (!near && *(int *)(d + 0x164) != 0) {
            *(int *)(d + 0x164) = 0;
            *(u128_DC920 *)(d + 0x150) = *(u128_DC920 *)(P + 0x50);
        }
        if (func_001F9908_DC920((int *)(d + 0x168)) != 0) {
            const float k = 0.0174532925f;
            float r, yaw, pitch;
            r = func_002140F8_DC920(180.0f, 300.0f);
            *(int *)(d + 0x168) = func_001FA898_DC920(func_001F9878_DC920(r));
            r = func_002140F8_DC920(-90.0f, 90.0f);
            yaw = func_001FA748_DC920(*(float *)(m + 0x48), r * k);
            pitch = func_002140F8_DC920(0.0f, 30.0f) * k;
            func_00215C00_DC920(d + 0x150, 6.0f, yaw, pitch);
            func_001F9BD8_DC920(d + 0x150, d + 0x150, m + 0x10);
        }
        if (*(int *)(d + 0x164) != 0) {
            loc = *(u128_DC920 *)(P + 0x50);
            f22 = 0.04f;
            f23 = 0.3f;
        } else {
            loc = *(u128_DC920 *)(d + 0x150);
        }
    }
    if (looked) {
        float yaw, pitch;
        *(u128_DC920 *)v2 = *(u128_DC920 *)(m + 0x10);
        v2[2] = v2[2] + 1.0f;
        func_001F9BF0_DC920(v3, &loc, v2);
        yaw = func_001FA790_DC920(func_L00_001FF860_DC920(v3[0], v3[1]), *(float *)(m + 0x48));
        pitch = -func_L00_001FF860_DC920(func_001F9CE8_DC920(v3), v3[2]);
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
        *(float *)(d + 0xA4) = pitch;
        *(float *)(d + 0x128) = yaw * 0.5f;
        *(float *)(d + 0xA8) = yaw * 0.5f;
    }
    if (D_0015EEB0_DC920 != 0) {
        *(float *)(d + 0xB0) = 2.75f;
    }
    func_L00_00263950_DC920(m, d + 0x40, 1, f22 * D_0015EE64_DC920, f23 * D_0015EE64_DC920);
    func_L00_00263950_DC920(m, d + 0xC0, 0, f22 * D_0015EE64_DC920, f23 * D_0015EE64_DC920);
}
