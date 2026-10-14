/* functional: medium. Per-frame hero-attachment update (hero struct at D_0013F450): runs the frame helpers, creates the two follower mobys (classes 0x4B4 and 0x47A) once, keeps them on the hero's joints, blends their colour with a sine of the frame timer and the hero's state colours, and (state 3) pulses the hero's light colour and starts the 0x19 effect. Constants and call order are from the assembly; the meaning of several hero fields is a guess. Not run against captured calls. */
extern char G_hero_E7B8[] __asm__("D_0013F450");
extern float G_17C0DC_E7B8 __asm__("D_L06_0017C0DC");
extern float G_17C44C_E7B8 __asm__("D_L06_0017C44C");
extern char G_EEF0_E7B8[] __asm__("D_0015EEF0");
extern char G_EEB0_E7B8[] __asm__("D_0015EEB0");
extern float G_EE60_E7B8 __asm__("D_0015EE60");
extern float G_EE6C_E7B8 __asm__("D_0015EE6C");
extern char G_D5DD_E7B8[] __asm__("D_0013D5DD");
extern int G_F6B0_E7B8 __asm__("D_L06_0015F6B0");

extern void func_L00_00232EF0_E7B8(void) __asm__("func_L00_00232EF0");
extern void func_L00_00211D28_E7B8(void) __asm__("func_L00_00211D28");
extern void func_L06_00223630_E7B8(void) __asm__("func_L06_00223630");
extern void func_L00_0020C568_E7B8(void) __asm__("func_L00_0020C568");
extern void func_L00_002162B8_E7B8(void) __asm__("func_L00_002162B8");
extern void func_L06_0023BBE8_E7B8(void) __asm__("func_L06_0023BBE8");
extern void func_L00_00217698_E7B8(void) __asm__("func_L00_00217698");
extern void func_L00_00205B00_E7B8(void) __asm__("func_L00_00205B00");
extern void func_L00_00209088_E7B8(void) __asm__("func_L00_00209088");
extern void func_L00_00208260_E7B8(void) __asm__("func_L00_00208260");
extern void func_L00_00208070_E7B8(void) __asm__("func_L00_00208070");
extern void func_L00_00205FF0_E7B8(void) __asm__("func_L00_00205FF0");
extern char *func_0020D348_E7B8(int) __asm__("func_0020D348");
extern void func_L00_00251E30_E7B8(void *) __asm__("func_L00_00251E30");
extern void func_L00_00250800_E7B8(void *, int, void *) __asm__("func_L00_00250800");
extern void func_L00_00208318_E7B8(void) __asm__("func_L00_00208318");
extern void func_0020DAF8_E7B8(void *, int, void *) __asm__("func_0020DAF8");
extern void func_001FA480_E7B8(void *, void *) __asm__("func_001FA480");
extern void func_001F9EC0_E7B8(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_00214D28_E7B8(void *, float, float) __asm__("func_00214D28");
extern float func_001FA748_E7B8(float, float) __asm__("func_001FA748");
extern int func_001F9938_E7B8(void *) __asm__("func_001F9938");
extern float func_001F9FA8_E7B8(float) __asm__("func_001F9FA8");
extern int func_001FA8A8_E7B8(int, int, float) __asm__("func_001FA8A8");
extern void func_001F9BC0_E7B8(void *) __asm__("func_001F9BC0");
extern void func_001FA218_E7B8(void *, void *) __asm__("func_001FA218");
extern void func_001FA540_E7B8(void *, void *, void *) __asm__("func_001FA540");
extern int func_001F9850_E7B8(int) __asm__("func_001F9850");
extern void func_00213DE0_E7B8(void *, int, int, int) __asm__("func_00213DE0");
extern int func_0022ED80_E7B8(void *, int, void *) __asm__("func_0022ED80");
extern void func_L00_00248EF8_E7B8(void *) __asm__("func_L00_00248EF8");
extern float func_001FA888_E7B8(int) __asm__("func_001FA888");
extern int func_001FA898_E7B8(float) __asm__("func_001FA898");
extern void func_L00_00217570_E7B8(int, int) __asm__("func_L00_00217570");
extern void func_L00_002091D8_E7B8(void) __asm__("func_L00_002091D8");
extern void func_L00_0025805C_E7B8(void *) __asm__("func_L00_0025805C");

void func_L06_0021E7B8_impl(void) __asm__("func_L06_0021E7B8");
void func_L06_0021E7B8_impl(void) {
    char *g = G_hero_E7B8;
    float v0[4] __attribute__((aligned(16)));
    float mat[16] __attribute__((aligned(16)));
    float v1[4] __attribute__((aligned(16)));
    float angles[4] __attribute__((aligned(16)));
    float rot[16] __attribute__((aligned(16)));
    char *m;
    int c1, c2, col;
    int i;
    float f;

    func_L00_00232EF0_E7B8();
    func_L00_00211D28_E7B8();
    func_L06_00223630_E7B8();
    if (*(unsigned char *)(g + 0x20A4) == 1) {
        func_L00_0020C568_E7B8();
        func_L00_002162B8_E7B8();
    }
    func_L06_0023BBE8_E7B8();
    if (*(unsigned char *)(g + 0x20A4) == 0) {
        return;
    }
    func_L00_00217698_E7B8();
    func_L00_00205B00_E7B8();
    if (*(unsigned char *)(g + 0x20A4) == 0) {
        return;
    }
    func_L00_00209088_E7B8();
    func_L00_00208260_E7B8();
    func_L00_00208070_E7B8();
    if (*(unsigned char *)(g + 0x20A4) == 1) {
        G_17C0DC_E7B8 = *(float *)(G_EEF0_E7B8 + 0x28);
        func_L00_00205FF0_E7B8();
        if (*(int *)(g + 0x1624) == 0) {
            m = func_0020D348_E7B8(0x4B4);
            *(char **)(g + 0x1624) = m;
            if (m != 0) {
                *(int *)(m + 0x90) = 0x801432D7;
                *(int *)(g + 0x2300) = 0x301432D7;
                m = *(char **)(g + 0x1624);
                qcopy(m + 0x10, g + 0x80);
                qcopy(m + 0x40, g + 0x90);
                *(short *)(m + 0x32) = 0x40;
                *(unsigned char *)(*(char **)(g + 0x1624) + 0x31) = 1;
                *(unsigned char *)(*(char **)(g + 0x1624) + 0x30) = 0;
                *(short *)(*(char **)(g + 0x1624) + 0x34) = 0;
                m = *(char **)(g + 0x1624);
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) * 1.7f;
                m = *(char **)(g + 0x1624);
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x10;
                func_L00_00251E30_E7B8(*(char **)(g + 0x1624));
            }
        }
        if ((((*(unsigned short *)(*(char **)(g + 0x2080) + 0x34)) ^ 1) & 1) != 0) {
            func_L00_00250800_E7B8(*(char **)(g + 0x2080), 7, v0);
            qcopy(g + 0x1D80, v0);
            func_L00_00208318_E7B8();
            if (*(int *)(g + 0x1624) != 0) {
                func_0020DAF8_E7B8(*(char **)(g + 0x2080), 7, mat);
                m = *(char **)(g + 0x1624);
                qcopy(m + 0x10, mat + 12);
                func_001FA480_E7B8(m + 0xC0, mat);
                m = *(char **)(g + 0x1624);
                v1[0] = 0.0f; v1[1] = 0.0f; v1[2] = 0.0f; v1[3] = 0.0f;
                v1[2] = 0.0256999992f;
                func_001F9EC0_E7B8(v1, v1, m + 0xC0);
                m = *(char **)(g + 0x1624);
                *(float *)(m + 0x18) = *(float *)(m + 0x18) - v1[2];
                func_L00_00251E30_E7B8(*(char **)(g + 0x1624));
                f = 1.7f;
                if (*(unsigned char *)(G_EEB0_E7B8 + 3) != 0) {
                    f = 3.1f;
                }
                if (*(short *)(g + 0x22FC) != 0) {
                    f = f * 1.4f;
                }
                m = *(char **)(g + 0x1624);
                func_00214D28_E7B8(m + 0x2C, *(float *)(*(char **)(m + 0x24) + 0x24) * f, 0.02f);
            }
        }
        *(float *)(g + 0x1628) = func_001FA748_E7B8(*(float *)(g + 0x1628), G_EE6C_E7B8 * 2.96705961f);
        c1 = 0x281414C8;
        c2 = 0x1E00004B;
        if (func_001F9938_E7B8(g + 0x22FC) == 0) {
            *(float *)(g + 0x1628) = func_001FA748_E7B8(*(float *)(g + 0x1628), G_EE6C_E7B8 * 12.2173042f);
            switch (*(short *)(g + 0x22FE)) {
            case 2:
                c1 = 0x2814C8C8; c2 = 0x1E004B4B;
                break;
            case 1:
                c1 = 0x2814C814; c2 = 0x1E004B00;
                break;
            case 3:
                c1 = 0x323232FA; c2 = 0x1E000078;
                break;
            case 4:
                c1 = 0x28F01414; c2 = 0x1E4B0000;
                break;
            }
        }
        f = func_001F9FA8_E7B8(*(float *)(g + 0x1628));
        c1 = func_001FA8A8_E7B8(c1, c2, f * 0.5f + 0.5f);
        if (*(int *)(g + 0x1624) != 0) {
            m = *(char **)(g + 0x1624);
            *(int *)(*(char **)(g + 0x1624) + 0x90) = func_001FA8A8_E7B8(*(int *)(m + 0x90), c1, G_EE60_E7B8 * 0.1f);
        }
        *(int *)(g + 0x2300) = func_001FA8A8_E7B8(*(int *)(g + 0x2300), c1, G_EE60_E7B8 * 0.1f);
        if (*(int *)(g + 0x1620) == 0) {
            m = func_0020D348_E7B8(0x47A);
            *(char **)(g + 0x1620) = m;
            if (m != 0) {
                qcopy(m + 0x10, g + 0x80);
                qcopy(m + 0x40, g + 0x90);
                *(short *)(m + 0x32) = 0x40;
                *(unsigned char *)(*(char **)(g + 0x1620) + 0x31) = 1;
                *(unsigned char *)(*(char **)(g + 0x1620) + 0x30) = 0;
                *(short *)(*(char **)(g + 0x1620) + 0x34) = 0;
                func_L00_00251E30_E7B8(*(char **)(g + 0x1620));
            }
        }
        if (*(int *)(g + 0x1620) != 0) {
            func_0020DAF8_E7B8(*(char **)(g + 0x2080), 5, rot);
            m = *(char **)(g + 0x1620);
            qcopy(m + 0x10, rot + 12);
            func_L00_00251E30_E7B8(m);
            func_001F9BC0_E7B8(angles);
            angles[2] = -*(float *)(g + 0x98);
            func_001FA218_E7B8(mat, angles);
            func_001FA540_E7B8(rot, rot, mat);
            func_001FA480_E7B8(*(char **)(g + 0x1620) + 0xC0, rot);
            *(long *)(*(char **)(g + 0x1620) + 0x38) = *(long *)(*(char **)(g + 0x2080) + 0x38);
            if (*(int *)(g + 0x2084) == 0x4F) {
                if (*(unsigned char *)(*(char **)(g + 0x1620) + 0x53) != 1) {
                    func_00213DE0_E7B8(*(char **)(g + 0x1620), 1, 0, func_001F9850_E7B8(10));
                }
            } else {
                if (*(unsigned char *)(*(char **)(g + 0x1620) + 0x53) != 0) {
                    func_00213DE0_E7B8(*(char **)(g + 0x1620), 0, 0, func_001F9850_E7B8(0x11));
                }
            }
        }
        if (*(int *)(g + 0x22C0) != 0) {
            func_0022ED80_E7B8(*(char **)(g + 0x22C0) + 0x10, 0, *(char **)(g + 0x2080));
            *(short *)(g + 0x22FE) = *(unsigned short *)(g + 0x22C0);
            *(short *)(g + 0x22FC) = func_001F9850_E7B8(0x39);
        }
        *(int *)(g + 0x22C0) = 0;
    }
    func_L00_00248EF8_E7B8(g + 0x80);
    if (*(unsigned char *)(g + 0x20A4) == 3) {
        int n, t;
        int pulse = 0x80;
        int r, gg, b;
        if (*(unsigned char *)(G_EEB0_E7B8 + 1) != 0) {
            G_17C44C_E7B8 = 1.9f;
        }
        n = func_001F9850_E7B8(0xAA);
        if (*(short *)(g + 0x22E0) == 2) {
            n = func_001F9850_E7B8(0x32);
        } else if (*(short *)(g + 0x22E0) == 1) {
            n = func_001F9850_E7B8(0x5A);
        }
        f = func_001FA888_E7B8(n);
        t = G_F6B0_E7B8 % n;
        f = func_001F9FA8_E7B8((((float)t / f) + ((float)t / f)) * 3.14159274f + -3.14159274f);
        c1 = func_001FA898_E7B8(f * 20.0f) + 0x32;
        c2 = func_001FA898_E7B8(f * 70.0f) + 0xB4;
        i = func_001FA898_E7B8(f * 10.0f) + 0x14;
        if (!(c1 < 0x100)) c1 = 0xFF;
        if (!(c2 < 0x100)) c2 = 0xFF;
        *(unsigned short *)(*(char **)(g + 0x2080) + 0x34) = *(unsigned short *)(*(char **)(g + 0x2080) + 0x34) | 0x10;
        if (*(short *)(g + 0x22E0) == 2) {
            col = (pulse << 24) | (i << 16) | ((c1 / 2) << 8) | c2;
        } else {
            col = (pulse << 24) | (i << 16) | (c2 << 8) | c1;
        }
        m = *(char **)(g + 0x2080);
        *(int *)(g + 0) = *(int *)(g + 0);
        *(int *)(*(char **)(g + 0x2080) + 0x90) = func_001FA8A8_E7B8(*(int *)(m + 0x90), col, 0.1f);
        for (i = 0; i < 3; i++) {
            func_L00_00250800_E7B8(*(char **)(g + 0x2080), i, g + 0x1D50 + i * 16);
        }
        m = *(char **)(g + 0x2080);
        pulse = 0x30;
        r = *(unsigned char *)(m + 0x90);
        b = *(unsigned char *)(m + 0x92);
        gg = *(unsigned char *)(m + 0x91);
        if (*(short *)(g + 0x22DE) != 0) {
            float a = func_001FA888_E7B8(*(short *)(g + 0x22DE));
            float d = func_001FA888_E7B8(0x12);
            a = a / d;
            pulse = func_001FA898_E7B8(a * (float)pulse);
        }
        *(int *)(g + 0x22E4) = (pulse << 24) | (b << 16) | (gg << 8) | r;
        func_L00_00208318_E7B8();
        *(short *)(g + 0x22E0) = 0;
        if (*(unsigned char *)(g + 0x20A4) == 3) {
            int s = *(int *)(g + 0x20B8);
            if (s == 0 || s == 0x1F) {
                *(int *)(g + 0x20B8) = 0;
                if (*(unsigned char *)(g + 0x12E7) != 0) {
                    if (*(int *)(g + 0x300) != 0) {
                        if (*(unsigned char *)(G_D5DD_E7B8 + 7) != 0) {
                            if (*(short *)(g + 0x22DE) == 0) {
                                func_L00_00217570_E7B8(0x19, 0);
                                *(short *)(g + 0x22DC) = 1;
                                *(short *)(g + 0x22DE) = 0x12;
                            }
                        }
                    }
                }
            } else if (*(short *)(g + 0x22DE) == 0) {
                func_L00_00217570_E7B8(0x19, 0);
                *(short *)(g + 0x22DC) = 1;
                *(short *)(g + 0x22DE) = 0x12;
            }
        }
    }
    func_L00_002091D8_E7B8();
    func_L00_0025805C_E7B8(*(char **)(g + 0x2080));
}
