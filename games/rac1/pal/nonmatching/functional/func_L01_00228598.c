/* functional: medium. Help-menu frame update (state g[0x20A4] 3 = the panel): runs the update chain, picks a random-based colour from sin/cos of D_L01_0015F6B0 mod rand and applies it to the hero's tint (w+0x90), updates the 3 slots, the fade colour (g+0x22E4) and the sound/timer trigger (22DC/22DE). Float/clamp order hand-traced; not run. */
extern unsigned char D_0013E633[];
extern unsigned char D_0013D5DD[];
extern unsigned char D_0015EEB0[] MACRO_ADDR;
extern int D_L01_0015F6B0 MACRO_ADDR;
extern int func_001F9850(int);
extern float D_0017C0C0_a[] __asm__("D_0017C0C0");
extern float func_001FA888(int);
extern int func_001FA898(float);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern void func_L00_00232EF0(void);
extern void func_L00_00211D28(void);
extern void func_L01_0022D340(void);
extern void func_L01_00243078(void);
extern void func_L00_00217698(void);
extern void func_L00_00205B00(void);
extern void func_L00_00209088(void);
extern void func_L00_00208260(void);
extern void func_L00_00208070(void);
extern void func_L00_00248EF8(void *);
extern void func_L00_00250800(void *, int, void *);
extern void func_L00_00208318(void);
extern void func_L00_00217570(int, int);
extern void func_L00_002091D8(void);
extern void func_L00_0025805C(void *);

void func_L01_00228598(void) {
    char *g = (char *)D_0013E633 + 0xE1D;
    char *w;
    int v, h, i, r16, r17, r18, c20, color;
    float f0, f20, ang;

    func_L00_00232EF0();
    func_L00_00211D28();
    func_L01_0022D340();
    func_L01_00243078();
    if (*(unsigned char *)(g + 0x20A4) == 0) return;
    func_L00_00217698();
    func_L00_00205B00();
    if (*(unsigned char *)(g + 0x20A4) == 0) return;
    func_L00_00209088();
    func_L00_00208260();
    func_L00_00208070();
    func_L00_00248EF8(g + 0x80);
    if (*(unsigned char *)(g + 0x20A4) == 3) {
        if (D_0015EEB0[1] != 0) {
            D_0017C0C0_a[3] = 1.9f;
        }
        c20 = 0x80;
        v = func_001F9850(0xAA);
        h = *(short *)(g + 0x22E0);
        if (h == 2) {
            v = func_001F9850(0x32);
        } else if (h == 1) {
            v = func_001F9850(0x5A);
        }
        f0 = func_001FA888(v);
        ang = (float)(D_L01_0015F6B0 % v) / f0;
        ang = ang + ang;
        ang = ang * 3.14159274f;
        f20 = func_001F9FA8(ang + -3.14159274f);
        r16 = func_001FA898(f20 * 20.0f) + 0x32;
        r17 = func_001FA898(f20 * 70.0f) + 0xB4;
        r18 = func_001FA898(f20 * 10.0f) + 0x14;
        w = *(char **)(g + 0x2080);
        *(unsigned short *)(w + 0x34) |= 0x10;
        if (r16 >= 0x100) r16 = 0xFF;
        if (r17 >= 0x100) r17 = 0xFF;
        if (*(short *)(g + 0x22E0) == 2) {
            color = (c20 << 24) | (r18 << 16) | (((r16 + (int)((unsigned int)r16 >> 31)) >> 1) << 8) | r17;
        } else {
            color = (c20 << 24) | (r18 << 16) | (r17 << 8) | r16;
        }
        w = *(char **)(g + 0x2080);
        *(int *)(w + 0x90) = func_001FA8A8(*(int *)(w + 0x90), color, 0.1f);
        for (i = 0; i < 3; i++) {
            func_L00_00250800(*(void **)(g + 0x2080), i, g + 0x1D50 + i * 0x10);
        }
        c20 = 0x30;
        w = *(char **)(g + 0x2080);
        r16 = *(unsigned char *)(w + 0x90);
        r18 = *(unsigned char *)(w + 0x92);
        h = *(short *)(g + 0x22DE);
        r17 = *(unsigned char *)(w + 0x91);
        if (h != 0) {
            f20 = func_001FA888(h);
            f20 = f20 / func_001FA888(0x12);
            c20 = func_001FA898(f20 * (float)c20);
        }
        *(int *)(g + 0x22E4) = (c20 << 24) | (r18 << 16) | (r17 << 8) | r16;
        func_L00_00208318();
        *(short *)(g + 0x22E0) = 0;
        if (*(unsigned char *)(g + 0x20A4) == 3) {
            v = *(int *)(g + 0x20B8);
            if (v != 0 && v != 0x1F) {
                if (*(short *)(g + 0x22DE) == 0) {
                    func_L00_00217570(0x19, 0);
                    *(short *)(g + 0x22DC) = 1;
                    *(short *)(g + 0x22DE) = 0x12;
                }
            } else {
                v = *(unsigned char *)(g + 0x12E7);
                *(int *)(g + 0x20B8) = 0;
                if (v != 0 && *(int *)(g + 0x300) != 0 && *(unsigned char *)(D_0013D5DD + 7) != 0 &&
                    *(short *)(g + 0x22DE) == 0) {
                    func_L00_00217570(0x19, 0);
                    *(short *)(g + 0x22DC) = 1;
                    *(short *)(g + 0x22DE) = 0x12;
                }
            }
        }
    }
    func_L00_002091D8();
    func_L00_0025805C(*(void **)(g + 0x2080));
}
