/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL func_L00_002D6610 (0x364 bytes): what the horny toad of Veldin (class
 * 749) does every tick before its states: it takes the hit record aimed at it, and
 * a hit that counts throws it off (state 12, the knocked-back flight) and takes its
 * health; then its senses (the target, the alert). Not a PS2 match.
 *
 * This is the decompilation's functional C for the function
 * (nonmatching/game/func_L00_002D6610.c) with one correction: whether the hit counts
 * is the damage resolver's fifth argument (1: no hit, or one still held off by the
 * cooldown), not its return value, which is the reaction to play. The functional C
 * tested the return value, so a wrench hit (reaction 1) never hurt the creature. The
 * decompilation's near miss for the same function (nonmatching/shared/) and ReRAC's
 * notes on the US release (crates/rc-game/src/moby_update/classes/units: "out5 != 1
 * and not dying: health -= damage") both read the fifth argument. */
extern char *func_L00_0025B478_6610(void *, int, int) __asm__("func_L00_0025B478");
extern int func_L00_0025B4D0_6610(void *, void *, void *, int, int *, float *, int, int) __asm__("func_L00_0025B4D0");
extern void func_L00_0025BBA0_6610(void *, float *, void *, void *, int) __asm__("func_L00_0025BBA0");
extern void func_L00_0025D5B0_6610(void *, void *, int, int, int, float) __asm__("func_L00_0025D5B0");
extern void func_L00_0025E4B0_6610(void *, void *) __asm__("func_L00_0025E4B0");
extern void func_L00_0025E590_6610(void *, void *) __asm__("func_L00_0025E590");
extern int func_L00_00260FB0_6610(float, void *, void *, int, int, void *, int) __asm__("func_L00_00260FB0");
extern int func_001F9938_6610(void *) __asm__("func_001F9938");
extern float func_001F9878_6610(float) __asm__("func_001F9878");
extern float D_L00_00161A40 MACRO_ADDR;
extern float D_L00_00161A44 MACRO_ADDR;
extern float D_L00_00161A48 MACRO_ADDR;
extern float D_L00_00161A4C MACRO_ADDR;
extern float D_L00_00161A58 MACRO_ADDR;
extern float D_L00_00161A5C MACRO_ADDR;

void func_L00_002D6610(void *a0) {
    char *a = (char *)a0;
    char *d;
    char *b;
    char *p;
    char *q;
    Vx vec;
    int i10;
    float f14 = 0.0f;
    float t18;
    float f20;
    float f0, f1, f2, f3, f5, f6;
    int r1, r2, v;
    unsigned short h;

    if (*(unsigned char *)(a + 0x20) == 0)
        return;
    d = *(char **)(a + 0x78);

    h = *(unsigned short *)(a + 0x34);
    if (D_L00_0015F6A8_2d3330 == 2) {
        a[0x31] = 0;
        h |= 1;
    } else {
        a[0x31] = 1;
        h &= 0xFFFE;
    }
    *(unsigned short *)(a + 0x34) = h;

    f20 = 0.0f;
    b = func_L00_0025B478_6610(a, 0x330000, 0);
    r1 = func_L00_0025B4D0_6610(a, b, d + 0x20, 0, &i10, &f14, 0, 4);
    (void)r1;

    if (i10 != 1 && *(unsigned char *)(a + 0x20) != 12 && f14 != 0.0f) {
        f6 = D_L00_00161A40 * D_0015EE70;
        f5 = D_L00_00161A44 * D_0015EE70;
        f3 = D_L00_00161A48 * D_0015EE6C;
        f2 = D_L00_00161A4C * D_0015EE6C;
        d[0xAD] = 0;
        f0 = *(float *)(d + 0x20) - f14;
        f1 = D_0015EE6C + D_0015EE6C;
        *(float *)(d + 0x80) = f6;
        *(float *)(d + 0x98) = 0.75f;
        *(float *)(d + 0x84) = f5;
        *(float *)(d + 0x88) = f3;
        *(float *)(d + 0x8C) = f2;
        *(float *)(d + 0x20) = f0;
        *(float *)(d + 0xBC) = f1;
        *(int *)(d + 0x94) = 41;
        *(int *)(d + 0x90) = 512;
        *(unsigned short *)(a + 0x34) &= 0xEFFF;

        vec = *(Vx *)(b + 0x10);
        func_L00_0025BBA0_6610(&vec, &t18, d + 0x88, d + 0x8C, 41);
        f20 = t18;
        r2 = func_002140B0(2);
        func_L00_0025D5B0_6610(a, d + 0x70, r2 + 6, 1, 0, f20);
        *(float *)(d + 0xC0) = D_L00_00161A5C;
        *(float *)(d + 0xC4) = D_L00_00161A58;
        *(unsigned char *)(a + 0x20) = 12;
        *(unsigned char *)(d + 0x67) = 0x78;
        func_L00_0025E4B0_6610(a, d + 0x60);
        func_L00_002584A8_2D19E8((u8 *)a, 0, -1);
    }
    *(unsigned char *)(a + 0xA4) = 0xFF;
    func_L00_0025E590_6610(a, d + 0x60);

    q = d + 0x180;
    if (*(unsigned char *)(a + 0x20) == 3) {
        p = D_L00_001B0830_2D19E8[*(int *)(d + 0x1E4)];
        func_L00_00260FB0_6610(64.0f, a, q, 0, 0, p + 0x10, *(int *)p);
    } else {
        p = D_L00_001B0830_2D19E8[*(int *)(d + 0x1E0)];
        func_L00_00260FB0_6610(24.0f, a, q, 0, 0, p + 0x10, *(int *)p);
    }
    v = *(int *)(d + 0x1C0);

    if (v == 0) {
        *(int *)(d + 0x1C0) = *(int *)(D_0013E633 + 0x2E9D);
        *(Vx *)q = *(Vx *)(D_0013E633 + 0xE9D);
    }

    func_001F9938_6610(d + 0x276);
    if (*(int *)(d + 0x38) != 0) {
        *(short *)(d + 0x276) = (short)func_001FA898_2D3F40(
            func_001F9878_6610(func_002140F8_2D19E8(180.0f, 240.0f)));
    }
    *(int *)(d + 0x38) = 0;

    if (*(unsigned char *)(a + 0x20) == 5 || *(unsigned char *)(a + 0x20) == 8) {
        if (*(short *)(d + 0x276) != 0) {
            *(unsigned char *)(a + 0x20) = 10;
            if (*(unsigned char *)(a + 0x53) != 4) {
                func_00213DE0(a, 4, 0, func_001F9850(20));
            }
        }
    }
}
