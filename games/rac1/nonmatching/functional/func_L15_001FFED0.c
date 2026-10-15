/* functional: medium. Per-frame hero update for the vehicle/ship modes: runs the common update calls, then for mode 2 scales a tuning float and records the moby's point 6 into the hero; for mode 3 pulses the moby's colour (sin of a frame counter), eases it toward the target, records the three moby points, builds the fade colour at hero+0x22E4 and starts the pending-item message (state 0x19). From the NTSC decomp's draft for the same function; the stores to 0x17C0EC/0x17C24C are tuning words of this overlay. */
extern unsigned char D_0013E633_FFED0[] __asm__("D_0013E633");
extern unsigned char D_0015EEB0_FFED0[] __asm__("D_0015EEB0");
extern unsigned char D_0013D5DD_FFED0[] __asm__("D_0013D5DD");
extern int D_L15_0015F6B0_FFED0 __asm__("D_L15_0015F6B0");
extern float D_GPF_FFED0 __asm__("D_0015EF18");
extern void c0_FFED0(void) __asm__("func_L00_00232EF0");
extern void c1_FFED0(void) __asm__("func_L00_00211D28");
extern void c2_FFED0(void) __asm__("func_L15_00204AC0");
extern void c3_FFED0(void) __asm__("func_L15_0021D7A8");
extern void c4_FFED0(void) __asm__("func_L00_00217698");
extern void c5_FFED0(void) __asm__("func_L00_00205B00");
extern void c6_FFED0(void) __asm__("func_L00_00209088");
extern void c7_FFED0(void) __asm__("func_L00_00208260");
extern void c8_FFED0(void) __asm__("func_L00_00208070");
extern void c9_FFED0(void *) __asm__("func_L00_00248EF8");
extern void c10_FFED0(void) __asm__("func_L00_00205FF0");
extern void c11_FFED0(void) __asm__("func_L00_00206808");
extern void pt_FFED0(void *, int, void *) __asm__("func_L00_00250800");
extern void c12_FFED0(void) __asm__("func_L00_00208318");
extern int period_FFED0(int) __asm__("func_001F9850");
extern float itof_FFED0(int) __asm__("func_001FA888");
extern float sin_FFED0(float) __asm__("func_001F9FA8");
extern int ftoi_FFED0(float) __asm__("func_001FA898");
extern unsigned int ease_FFED0(float, unsigned int, unsigned int) __asm__("func_001FA8A8");
extern void msg_FFED0(int, int) __asm__("func_L00_00217570");
extern void c13_FFED0(void) __asm__("func_L00_002091D8");
extern void c14_FFED0(void *) __asm__("func_L00_0025805C");

void func_L15_001FFED0(void) {
    unsigned char *base = D_0013E633_FFED0 + 0xE1D;
    unsigned char *moby;
    unsigned int copy[4] __attribute__((aligned(16)));
    int period, red, green, blue, alpha, i;
    unsigned int color;
    int rb, gb, bb;
    float pulse;

    c0_FFED0();
    c1_FFED0();
    c2_FFED0();
    c3_FFED0();
    if (base[0x20A4] == 0)
        return;
    c4_FFED0();
    c5_FFED0();
    if (base[0x20A4] == 0)
        return;
    c6_FFED0();
    c7_FFED0();
    c8_FFED0();
    c9_FFED0(base + 0x80);

    if (base[0x20A4] == 2) {
        if (D_0015EEB0_FFED0[3] != 0)
            *(float *)0x17C0EC = D_GPF_FFED0 * 1.4f;
        c10_FFED0();
        c11_FFED0();
        moby = *(unsigned char **)(base + 0x2080);
        if (((*(unsigned short *)(moby + 0x34) ^ 1) & 1) != 0) {
            pt_FFED0(moby, 6, copy);
            *(unsigned long long __attribute__((mode(TI))) *)(base + 0x1D80) = *(unsigned long long __attribute__((mode(TI))) *)copy;
            c12_FFED0();
        }
    }

    if (base[0x20A4] == 3) {
        if (D_0015EEB0_FFED0[1] != 0)
            *(unsigned int *)0x17C24C = 0x3FF33333;
        alpha = 0x80;
        period = period_FFED0(0xAA);
        if (*(short *)(base + 0x22E0) == 2)
            period = period_FFED0(0x32);
        else if (*(short *)(base + 0x22E0) == 1)
            period = period_FFED0(0x5A);
        pulse = sin_FFED0((((float)(D_L15_0015F6B0_FFED0 % period) / itof_FFED0(period)) * 2.0f) * 3.1415927f + -3.1415927f);
        red = ftoi_FFED0(pulse * 20.0f) + 0x32;
        green = ftoi_FFED0(pulse * 70.0f) + 0xB4;
        blue = ftoi_FFED0(pulse * 10.0f) + 0x14;
        moby = *(unsigned char **)(base + 0x2080);
        if (red > 0xFF) red = 0xFF;
        if (green > 0xFF) green = 0xFF;
        *(unsigned short *)(moby + 0x34) |= 0x10;
        if (*(short *)(base + 0x22E0) == 2)
            color = (alpha << 24) | (blue << 16) | ((red / 2) << 8) | green;
        else
            color = (alpha << 24) | (blue << 16) | (green << 8) | red;
        *(unsigned int *)(*(unsigned char **)(base + 0x2080) + 0x90) =
            ease_FFED0(0.1f, *(unsigned int *)(*(unsigned char **)(base + 0x2080) + 0x90), color);
        for (i = 0; i < 3; i++)
            pt_FFED0(*(void **)(base + 0x2080), i, base + 0x1D50 + i * 16);
        alpha = 0x30;
        moby = *(unsigned char **)(base + 0x2080);
        rb = moby[0x90];
        gb = moby[0x91];
        bb = moby[0x92];
        if (*(short *)(base + 0x22DE) != 0)
            alpha = ftoi_FFED0((itof_FFED0(*(short *)(base + 0x22DE)) / itof_FFED0(0x12)) * (float)alpha);
        *(unsigned int *)(base + 0x22E4) = (alpha << 24) | (bb << 16) | (gb << 8) | rb;
        c12_FFED0();
        *(short *)(base + 0x22E0) = 0;
        if (base[0x20A4] == 3) {
            int pending = *(int *)(base + 0x20B8);
            if (pending != 0 && pending != 0x1F) {
                if (*(short *)(base + 0x22DE) == 0) {
                    msg_FFED0(0x19, 0);
                    *(short *)(base + 0x22DC) = 1;
                    *(short *)(base + 0x22DE) = 0x12;
                }
            } else {
                *(int *)(base + 0x20B8) = 0;
                if (base[0x12E7] != 0 && *(int *)(base + 0x300) != 0 && D_0013D5DD_FFED0[7] != 0 &&
                    *(short *)(base + 0x22DE) == 0) {
                    msg_FFED0(0x19, 0);
                    *(short *)(base + 0x22DC) = 1;
                    *(short *)(base + 0x22DE) = 0x12;
                }
            }
        }
    }
    c13_FFED0();
    c14_FFED0(*(void **)(base + 0x2080));
}
