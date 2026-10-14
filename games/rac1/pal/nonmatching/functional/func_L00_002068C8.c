/* functional: medium. Hero per-frame update for the help-screen overlay: runs the common hero steps, then per vehicle mode 1 (body, glow and shadow mobys, colours), 2 and 3 (pulsing colour, fade); written from the asm with the NTSC decomp draft as a guide, the PAL address-shifted draft layout was checked line by line. */
extern void func_L00_00232EF0_068C8(void) __asm__("func_L00_00232EF0");
extern void func_L00_00211D28_068C8(void) __asm__("func_L00_00211D28");
extern void func_L00_0020BFD8_068C8(void) __asm__("func_L00_0020BFD8");
extern void func_L00_0020C568_068C8(void) __asm__("func_L00_0020C568");
extern void func_L00_002162B8_068C8(void) __asm__("func_L00_002162B8");
extern void func_L00_0022A340_068C8(void) __asm__("func_L00_0022A340");
extern void func_L00_00217698_068C8(void) __asm__("func_L00_00217698");
extern void func_L00_00205B00_068C8(void) __asm__("func_L00_00205B00");
extern void func_L00_00209088_068C8(void) __asm__("func_L00_00209088");
extern void func_L00_00208260_068C8(void) __asm__("func_L00_00208260");
extern void func_L00_00208070_068C8(void) __asm__("func_L00_00208070");
extern void func_L00_00205FF0_068C8(void) __asm__("func_L00_00205FF0");
extern void func_L00_00206808_068C8(void) __asm__("func_L00_00206808");
extern void func_L00_00208318_068C8(void) __asm__("func_L00_00208318");
extern void func_L00_002091D8_068C8(void) __asm__("func_L00_002091D8");
extern void func_L00_0025805C_068C8(void *) __asm__("func_L00_0025805C");
extern void func_L00_00248EF8_068C8(void *) __asm__("func_L00_00248EF8");
extern void func_L00_00251E30_068C8(void *) __asm__("func_L00_00251E30");
extern void func_L00_00250800_068C8(void *, int, void *) __asm__("func_L00_00250800");
extern void func_L00_00217570_068C8(int, int) __asm__("func_L00_00217570");
extern char *func_0020D348_068C8(int) __asm__("func_0020D348");
extern void func_0020DAF8_068C8(void *, int, void *) __asm__("func_0020DAF8");
extern void func_001FA480_068C8(void *, void *) __asm__("func_001FA480");
extern void func_001F9EC0_068C8(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_00214D28_068C8(float, float, void *) __asm__("func_00214D28");
extern float func_001FA748_068C8(float, float) __asm__("func_001FA748");
extern int func_001F9938_068C8(void *) __asm__("func_001F9938");
extern float func_001F9FA8_068C8(float) __asm__("func_001F9FA8");
extern int func_001FA8A8_068C8(float, int, int) __asm__("func_001FA8A8");
extern void func_001F9BC0_068C8(void *) __asm__("func_001F9BC0");
extern void func_001FA218_068C8(void *, void *) __asm__("func_001FA218");
extern void func_001FA540_068C8(void *, void *, void *) __asm__("func_001FA540");
extern int func_001F9850_068C8(int) __asm__("func_001F9850");
extern void func_00213DE0_068C8(void *, int, int, int) __asm__("func_00213DE0");
extern void func_0022ED80_068C8(void *, int, void *) __asm__("func_0022ED80");
extern float func_001FA888_068C8(int) __asm__("func_001FA888");
extern int func_001FA898_068C8(float) __asm__("func_001FA898");
extern char D_0013F450_068C8[] __asm__("D_0013F450");
extern unsigned char D_0015EEB0_068C8[] __asm__("D_0015EEB0") MACRO_ADDR;
extern float D_0015EF18_068C8 __asm__("D_0015EF18") MACRO_ADDR;
extern float D_0015EE6C_068C8 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE60_068C8 __asm__("D_0015EE60") MACRO_ADDR;
extern int D_L00_0015F6B0_068C8 __asm__("D_L00_0015F6B0") MACRO_ADDR;
extern char D_L00_0017B95C_068C8[] __asm__("D_L00_0017B95C") NOT_SDA;
extern unsigned char D_0013D5DD_068C8[] __asm__("D_0013D5DD") NOT_SDA;

void func_L00_002068C8(void) {
    long buf[20] __attribute__((aligned(16)));
    char *g = D_0013F450_068C8;
    char *m;
    char *m2;
    unsigned int color, color2;
    float scale, wave, f;
    int span, red, green, blue, alpha, i;

    func_L00_00232EF0_068C8();
    func_L00_00211D28_068C8();
    func_L00_0020BFD8_068C8();
    if (*(unsigned char *)(g + 0x20A4) == 1) {
        func_L00_0020C568_068C8();
        func_L00_002162B8_068C8();
    }
    func_L00_0022A340_068C8();
    if (*(unsigned char *)(g + 0x20A4) == 0) return;
    func_L00_00217698_068C8();
    func_L00_00205B00_068C8();
    if (*(unsigned char *)(g + 0x20A4) == 0) return;
    func_L00_00209088_068C8();
    func_L00_00208260_068C8();
    func_L00_00208070_068C8();
    if (*(unsigned char *)(g + 0x20A4) == 1) {
        *(float *)D_L00_0017B95C_068C8 = D_0015EF18_068C8;
        func_L00_00205FF0_068C8();
        if (*(char **)(g + 0x1624) == 0) {
            m = func_0020D348_068C8(0x4B4);
            *(char **)(g + 0x1624) = m;
            if (m != 0) {
                *(unsigned int *)(m + 0x90) = 0x801432D7;
                *(unsigned int *)(g + 0x2300) = 0x301432D7;
                m = *(char **)(g + 0x1624);
                qcopy(m + 0x10, g + 0x80);
                qcopy(m + 0x40, g + 0x90);
                *(short *)(m + 0x32) = 0x40;
                (*(char **)(g + 0x1624))[0x31] = 1;
                (*(char **)(g + 0x1624))[0x30] = 0;
                *(short *)(*(char **)(g + 0x1624) + 0x34) = 0;
                m = *(char **)(g + 0x1624);
                *(float *)(m + 0x2C) = *(float *)(m + 0x2C) * 1.7f;
                m = *(char **)(g + 0x1624);
                *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x10;
                func_L00_00251E30_068C8(*(char **)(g + 0x1624));
            }
        }
        if (((*(unsigned short *)(*(char **)(g + 0x2080) + 0x34)) ^ 1) & 1) {
            func_L00_00250800_068C8(*(char **)(g + 0x2080), 7, buf);
            qcopy(g + 0x1D80, buf);
            func_L00_00208318_068C8();
            if (*(char **)(g + 0x1624) != 0) {
                func_0020DAF8_068C8(*(char **)(g + 0x2080), 7, (char *)buf + 0x10);
                m = *(char **)(g + 0x1624);
                qcopy(m + 0x10, (char *)buf + 0x40);
                func_001FA480_068C8(m + 0xC0, (char *)buf + 0x10);
                m = *(char **)(g + 0x1624);
                buf[10] = 0;
                buf[11] = 0;
                *(float *)((char *)buf + 0x58) = 0.0257f;
                func_001F9EC0_068C8((char *)buf + 0x50, (char *)buf + 0x50, m + 0xC0);
                m = *(char **)(g + 0x1624);
                *(float *)(m + 0x18) = *(float *)(m + 0x18) - *(float *)((char *)buf + 0x58);
                func_L00_00251E30_068C8(*(char **)(g + 0x1624));
                scale = 1.7f;
                if (D_0015EEB0_068C8[3] != 0) scale = 3.1f;
                if (*(short *)(g + 0x22FC) != 0) scale = scale * 1.4f;
                m = *(char **)(g + 0x1624);
                func_00214D28_068C8(*(float *)(*(char **)(m + 0x24) + 0x24) * scale, 0.02f, m + 0x2C);
            }
        }
        color = 0x281414C8;
        color2 = 0x1E00004B;
        *(float *)(g + 0x1628) = func_001FA748_068C8(*(float *)(g + 0x1628), D_0015EE6C_068C8 * 2.9670597f);
        if (func_001F9938_068C8(g + 0x22FC) == 0) {
            *(float *)(g + 0x1628) = func_001FA748_068C8(*(float *)(g + 0x1628), D_0015EE6C_068C8 * 12.217304f);
            i = *(short *)(g + 0x22FE);
            if (i == 1) { color = 0x2814C814; color2 = 0x1E004B00; }
            else if (i == 2) { color = 0x2814C8C8; color2 = 0x1E004B4B; }
            else if (i == 3) { color = 0x323232FA; color2 = 0x1E000078; }
            else if (i == 4) { color = 0x28F01414; color2 = 0x1E4B0000; }
        }
        wave = func_001F9FA8_068C8(*(float *)(g + 0x1628));
        color = func_001FA8A8_068C8(wave * 0.5f + 0.5f, color, color2);
        m = *(char **)(g + 0x1624);
        if (m != 0) {
            *(int *)(m + 0x90) = func_001FA8A8_068C8(D_0015EE60_068C8 * 0.1f, *(int *)(m + 0x90), color);
        }
        *(int *)(g + 0x2300) = func_001FA8A8_068C8(D_0015EE60_068C8 * 0.1f, *(int *)(g + 0x2300), color);
        if (*(char **)(g + 0x1620) == 0) {
            m = func_0020D348_068C8(0x47A);
            *(char **)(g + 0x1620) = m;
            if (m != 0) {
                qcopy(m + 0x10, g + 0x80);
                qcopy(m + 0x40, g + 0x90);
                *(short *)(m + 0x32) = 0x40;
                (*(char **)(g + 0x1620))[0x31] = 1;
                (*(char **)(g + 0x1620))[0x30] = 0;
                *(short *)(*(char **)(g + 0x1620) + 0x34) = 0;
                func_L00_00251E30_068C8(*(char **)(g + 0x1620));
            }
        }
        if (*(char **)(g + 0x1620) != 0) {
            func_0020DAF8_068C8(*(char **)(g + 0x2080), 5, buf);
            qcopy(*(char **)(g + 0x1620) + 0x10, (char *)buf + 0x30);
            func_L00_00251E30_068C8(*(char **)(g + 0x1620));
            func_001F9BC0_068C8((char *)buf + 0x40);
            *(float *)((char *)buf + 0x48) = -*(float *)(g + 0x98);
            func_001FA218_068C8((char *)buf + 0x60, (char *)buf + 0x40);
            func_001FA540_068C8(buf, buf, (char *)buf + 0x60);
            func_001FA480_068C8(*(char **)(g + 0x1620) + 0xC0, buf);
            m = *(char **)(g + 0x2080);
            m2 = *(char **)(g + 0x1620);
            *(long *)(m2 + 0x38) = *(long *)(m + 0x38);
            m2 = *(char **)(g + 0x1620);
            if (*(int *)(g + 0x2084) == 0x4F) {
                if (*(unsigned char *)(m2 + 0x53) != 1) {
                    i = func_001F9850_068C8(10);
                    func_00213DE0_068C8(*(char **)(g + 0x1620), 1, 0, i);
                }
            } else if (*(unsigned char *)(m2 + 0x53) != 0) {
                i = func_001F9850_068C8(0x11);
                func_00213DE0_068C8(*(char **)(g + 0x1620), 0, 0, i);
            }
        }
        if (*(char **)(g + 0x22C0) != 0) {
            func_0022ED80_068C8(*(char **)(g + 0x22C0) + 0x10, 0, *(char **)(g + 0x2080));
            *(short *)(g + 0x22FE) = (short)*(unsigned short *)(g + 0x22C0);
            *(short *)(g + 0x22FC) = (short)func_001F9850_068C8(0x39);
        }
        *(int *)(g + 0x22C0) = 0;
    }
    func_L00_00248EF8_068C8(g + 0x80);
    if (*(unsigned char *)(g + 0x20A4) == 2) {
        if (D_0015EEB0_068C8[3] != 0) {
            *(float *)(D_L00_0017B95C_068C8 + 0x210) = D_0015EF18_068C8 * 1.4f;
        }
        func_L00_00205FF0_068C8();
        func_L00_00206808_068C8();
        if (((*(unsigned short *)(*(char **)(g + 0x2080) + 0x34)) ^ 1) & 1) {
            func_L00_00250800_068C8(*(char **)(g + 0x2080), 6, buf);
            qcopy(g + 0x1D80, buf);
            func_L00_00208318_068C8();
        }
    }
    if (*(unsigned char *)(g + 0x20A4) == 3) {
        if (D_0015EEB0_068C8[1] != 0) {
            *(float *)(D_L00_0017B95C_068C8 + 0x370) = 1.9f;
        }
        span = func_001F9850_068C8(0xAA);
        alpha = 0x80;
        if (*(short *)(g + 0x22E0) == 2) {
            span = func_001F9850_068C8(0x32);
        } else if (*(short *)(g + 0x22E0) == 1) {
            span = func_001F9850_068C8(0x5A);
        }
        f = func_001FA888_068C8(span);
        f = (float)(D_L00_0015F6B0_068C8 % span) / f;
        wave = func_001F9FA8_068C8((f + f) * 3.1415927f + -3.1415927f);
        red = func_001FA898_068C8(wave * 20.0f) + 0x32;
        green = func_001FA898_068C8(wave * 70.0f) + 0xB4;
        blue = func_001FA898_068C8(wave * 10.0f) + 0x14;
        if (red >= 0x100) red = 0xFF;
        if (green >= 0x100) green = 0xFF;
        m = *(char **)(g + 0x2080);
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) | 0x10;
        if (*(short *)(g + 0x22E0) == 2) {
            color = ((unsigned int)alpha << 24) | ((unsigned int)blue << 16) | ((unsigned int)(red / 2) << 8) | (unsigned int)green;
        } else {
            color = ((unsigned int)alpha << 24) | ((unsigned int)blue << 16) | ((unsigned int)green << 8) | (unsigned int)red;
        }
        m = *(char **)(g + 0x2080);
        *(int *)(m + 0x90) = func_001FA8A8_068C8(0.1f, *(int *)(m + 0x90), color);
        for (i = 0; i < 3; i++) {
            func_L00_00250800_068C8(*(char **)(g + 0x2080), i, g + 0x1D50 + i * 0x10);
        }
        m = *(char **)(g + 0x2080);
        alpha = 0x30;
        red = *(unsigned char *)(m + 0x90);
        blue = *(unsigned char *)(m + 0x92);
        green = *(unsigned char *)(m + 0x91);
        if (*(short *)(g + 0x22DE) != 0) {
            f = func_001FA888_068C8(*(short *)(g + 0x22DE));
            f = f / func_001FA888_068C8(0x12);
            alpha = func_001FA898_068C8(f * (float)alpha);
        }
        *(unsigned int *)(g + 0x22E4) = ((unsigned int)alpha << 24) | ((unsigned int)blue << 16) | ((unsigned int)green << 8) | (unsigned int)red;
        func_L00_00208318_068C8();
        *(short *)(g + 0x22E0) = 0;
        if (*(unsigned char *)(g + 0x20A4) == 3) {
            if (*(int *)(g + 0x20B8) != 0 && *(int *)(g + 0x20B8) != 0x1F) {
                if (*(short *)(g + 0x22DE) == 0) {
                    func_L00_00217570_068C8(0x19, 0);
                    *(short *)(g + 0x22DC) = 1;
                    *(short *)(g + 0x22DE) = 0x12;
                }
            } else {
                *(int *)(g + 0x20B8) = 0;
                if (*(unsigned char *)(g + 0x12E7) != 0 && *(int *)(g + 0x300) != 0 && D_0013D5DD_068C8[7] != 0 && *(short *)(g + 0x22DE) == 0) {
                    func_L00_00217570_068C8(0x19, 0);
                    *(short *)(g + 0x22DC) = 1;
                    *(short *)(g + 0x22DE) = 0x12;
                }
            }
        }
    }
    func_L00_002091D8_068C8();
    func_L00_0025805C_068C8(*(char **)(g + 0x2080));
}
