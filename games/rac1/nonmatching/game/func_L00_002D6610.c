/* fcheck: unchecked (level/overlay function; fcheck runs boot-executable functions only; OpenRAC tools/fcheck_gate.py, 2026-10-18). */
/* Veldin level code: per-frame step of a moby's state 0x20 (retail func_L00_002D6610, 0x364 bytes).
 * Runs a sub-state machine on a[0x20] (3, 5, 8, 10, 12); when a[0x20] is 12 or the sweep finds
 * nothing it only stores 0xFF into a[0xA4]. Globals at gp-0x52xx are read as floats. */
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

    /* Whether the hit counts is the resolver's fifth argument (1: none, or one still held off);
       its return value is the reaction to play, 1 for a wrench hit. */
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
