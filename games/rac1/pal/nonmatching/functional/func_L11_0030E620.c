/* functional: high. Same reads, writes, calls and arguments in the same order as retail (no COP2/VU0 code; the lq/sq pairs are 16-byte copies through qcopy); differs only in compiler choices (registers, stack slots, which global floats are kept in registers or reloaded, branch layout), and the gp-relative globals are named by their addresses (_gp = 0x166D00). */
/* func_L11_0030E620 -- src/overlays/l11_pokitaru/vendor_002CC828.c (functional C for the port, not a match)
 * Pose of the cutscene's helper mobys (classes 0x480-0x483, spawned by func_L11_0030DBF8, which calls
 * this in its states 0 and 3), driven by the eased progress values at D_L11_00161F54..F68:
 * - the six D_L11_001DA9E0 mobys (two sides of three, mirrored by the sign s) and the four
 *   D_L11_00161FB0 mobys take the controller's rotation, turned by s*pi/2 in z, a fixed tilt
 *   (D_L11_00161F4C degrees) and a fanned x angle opened by 0x161F48*(1-0x161F54); each is placed at
 *   the controller (+14 along its rotation, +0x161F3C up) plus its own offset vector (+0xE0) and the
 *   controller's side vector (+0xD0) times s; the 0x161FB0 ones also turn their x angle by the
 *   0x161F58/0x161F5C progress;
 * - the four D_L11_00161FC0 mobys follow the 0x161FB0 ones (offset from their +0xE0/+0xD0 vectors),
 *   tilted by 0x161F94 degrees times the 0x161F60/0x161F64 progress;
 * - the four D_L11_00161FA0 mobys sit on either side of the controller, spread out by
 *   1-0x161F68 (at most 20), at height 0x161F98, with the controller's rotation.
 * Every moby is then refreshed with func_L00_00251E30.
 * equiv: DIFFERENT only in compiler choices, each read by hand: the +-1 selects built with slt/movn
 * instead of slti/movz (same values), branch polarity, and which call a leftover argument register or
 * a reloaded mobys-array pointer is credited to (same number of loads of each array). */

extern char *D_L11_001DA9E0_30E620[6] __asm__("D_L11_001DA9E0");
extern char *D_L11_00161FB0_30E620[4] __asm__("D_L11_00161FB0") MACRO_ADDR;
extern char *D_L11_00161FC0_30E620[4] __asm__("D_L11_00161FC0") MACRO_ADDR;
extern char *D_L11_00161FA0_30E620[4] __asm__("D_L11_00161FA0") MACRO_ADDR;

extern float D_L11_00161F3C_30E620 SDATA(D_L11_00161F3C);
extern float D_L11_00161F40_30E620 SDATA(D_L11_00161F40);
extern float D_L11_00161F44_30E620 SDATA(D_L11_00161F44);
extern float D_L11_00161F48_30E620 SDATA(D_L11_00161F48);
extern float D_L11_00161F4C_30E620 SDATA(D_L11_00161F4C);
extern float D_L11_00161F54_30E620 SDATA(D_L11_00161F54);
extern float D_L11_00161F58_30E620 SDATA(D_L11_00161F58);
extern float D_L11_00161F5C_30E620 SDATA(D_L11_00161F5C);
extern float D_L11_00161F60_30E620 SDATA(D_L11_00161F60);
extern float D_L11_00161F64_30E620 SDATA(D_L11_00161F64);
extern float D_L11_00161F68_30E620 SDATA(D_L11_00161F68);
extern float D_L11_00161F8C_30E620 SDATA(D_L11_00161F8C);
extern float D_L11_00161F90_30E620 SDATA(D_L11_00161F90);
extern float D_L11_00161F94_30E620 SDATA(D_L11_00161F94);
extern float D_L11_00161F98_30E620 SDATA(D_L11_00161F98);

extern float func_001FA748_30E620(float, float) __asm__("func_001FA748");
extern float func_001FA790_30E620(float, float) __asm__("func_001FA790");
extern void func_001FA1F8_30E620(void *, void *) __asm__("func_001FA1F8");
extern void func_001F9BC0_30E620(void *) __asm__("func_001F9BC0");
extern void func_001F9BD8_30E620(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_L00_001FF4B0_30E620(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_L00_00251E30_30E620(void *) __asm__("func_L00_00251E30");

#define F_30E620(p, off) (*(float *)((char *)(p) + (off)))

void func_L11_0030E620_m(char *moby) __asm__("func_L11_0030E620");
void func_L11_0030E620_m(char *moby) {
    float a[4] __attribute__((aligned(16)));
    float b[4] __attribute__((aligned(16)));
    float c[4] __attribute__((aligned(16)));
    int i;

    for (i = 0; i < 6; i++) {
        int si = i < 3 ? -1 : 1;
        float s = (float)si;
        float base = (D_L11_00161F48_30E620 * (1.0f - D_L11_00161F54_30E620) + 30.0f) * 0.017453292f;
        float ang = (float)(i % 3) * 0.52359879f - base;
        char *m = D_L11_001DA9E0_30E620[i];

        qcopy(m + 0x40, moby + 0x40);
        F_30E620(D_L11_001DA9E0_30E620[i], 0x48) = func_001FA748_30E620(F_30E620(m, 0x48), s * 1.5707964f);
        F_30E620(D_L11_001DA9E0_30E620[i], 0x44) = D_L11_00161F4C_30E620 * 0.017453292f;
        F_30E620(D_L11_001DA9E0_30E620[i], 0x40) = ang;
        m = D_L11_001DA9E0_30E620[i];
        func_001FA1F8_30E620(m + 0xC0, m + 0x40);
        func_001F9BC0_30E620(a);
        func_L00_001FF4B0_30E620(a, moby + 0xC0, 14.0f);
        a[2] = a[2] + D_L11_00161F3C_30E620;
        func_001F9BD8_30E620(a, a, moby + 0x10);
        func_L00_001FF4B0_30E620(b, D_L11_001DA9E0_30E620[i] + 0xE0, D_L11_00161F40_30E620);
        func_L00_001FF4B0_30E620(c, moby + 0xD0, s * D_L11_00161F44_30E620);
        func_001F9BD8_30E620(b, b, c);
        func_001F9BD8_30E620(b, b, a);
        m = D_L11_001DA9E0_30E620[i];
        qcopy(m + 0x10, b);
        func_L00_00251E30_30E620(m);
    }

    for (i = 0; i < 4; i++) {
        int t = (i & 1) ? -1 : 1;
        int si = i < 2 ? -1 : 1;
        float s = (float)si;
        float base = (D_L11_00161F48_30E620 * (1.0f - D_L11_00161F54_30E620) + 15.0f) * 0.017453292f;
        float ang = (float)(i % 2) * 0.52359879f - base;
        float r;
        char *m = D_L11_00161FB0_30E620[i];

        qcopy(m + 0x40, moby + 0x40);
        F_30E620(D_L11_00161FB0_30E620[i], 0x48) = func_001FA748_30E620(F_30E620(m, 0x48), s * 1.5707964f);
        F_30E620(D_L11_00161FB0_30E620[i], 0x44) = D_L11_00161F4C_30E620 * 0.017453292f;
        F_30E620(D_L11_00161FB0_30E620[i], 0x40) = ang;
        m = D_L11_00161FB0_30E620[i];
        func_001FA1F8_30E620(m + 0xC0, m + 0x40);
        func_001F9BC0_30E620(a);
        func_L00_001FF4B0_30E620(a, moby + 0xC0, 14.0f);
        a[2] = a[2] + D_L11_00161F3C_30E620;
        func_001F9BD8_30E620(a, a, moby + 0x10);
        func_L00_001FF4B0_30E620(b, D_L11_00161FB0_30E620[i] + 0xE0, D_L11_00161F40_30E620);
        func_L00_001FF4B0_30E620(c, moby + 0xD0, s * D_L11_00161F44_30E620);
        func_001F9BD8_30E620(b, b, c);
        func_001F9BD8_30E620(b, b, a);
        qcopy(D_L11_00161FB0_30E620[i] + 0x10, b);

        r = func_001FA790_30E620((float)t * 0.2617994f, 1.3962634f);
        r = r * (t == -1 ? D_L11_00161F58_30E620 : D_L11_00161F5C_30E620);
        r = func_001FA748_30E620(r, 1.3962634f);
        m = D_L11_00161FB0_30E620[i];
        F_30E620(D_L11_00161FB0_30E620[i], 0x40) = func_001FA748_30E620(F_30E620(m, 0x40), r);
        func_L00_00251E30_30E620(D_L11_00161FB0_30E620[i]);
    }

    for (i = 0; i < 4; i++) {
        float k;

        func_L00_001FF4B0_30E620(a, D_L11_00161FB0_30E620[i] + 0xE0, D_L11_00161F8C_30E620);
        func_L00_001FF4B0_30E620(b, D_L11_00161FB0_30E620[i] + 0xD0, D_L11_00161F90_30E620);
        func_001F9BD8_30E620(a, a, b);
        func_001F9BD8_30E620(D_L11_00161FC0_30E620[i] + 0x10, D_L11_00161FB0_30E620[i] + 0x10, a);
        qcopy(D_L11_00161FC0_30E620[i] + 0x40, D_L11_00161FB0_30E620[i] + 0x40);
        k = (i & 1) ? D_L11_00161F60_30E620 : D_L11_00161F64_30E620;
        F_30E620(D_L11_00161FC0_30E620[i], 0x44) = func_001FA748_30E620(
            F_30E620(D_L11_00161FC0_30E620[i], 0x44), -(D_L11_00161F94_30E620 * 0.017453292f) * k);
        func_L00_00251E30_30E620(D_L11_00161FC0_30E620[i]);
    }

    for (i = 0; i < 4; i++) {
        float sign = i < 2 ? -1.0f : 1.0f;
        char *m;
        float d = (float)(i & 1) * 7.5f + 3.75f;

        d = d + (1.0f - D_L11_00161F68_30E620) * 13.25f;
        if (20.0f < d) {
            d = 20.0f;
        }
        func_L00_001FF4B0_30E620(a, moby + 0xC0, d * sign + 14.0f);
        a[2] = D_L11_00161F98_30E620;
        func_001F9BD8_30E620(D_L11_00161FA0_30E620[i] + 0x10, a, moby + 0x10);
        m = D_L11_00161FA0_30E620[i];
        qcopy(m + 0x40, moby + 0x40);
        func_L00_00251E30_30E620(m);
    }
}
#undef F_30E620
