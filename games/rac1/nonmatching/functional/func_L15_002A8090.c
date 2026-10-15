/* func_L15_002A8090 -- src/overlays/shared/vendor_00298BB8.c (functional C for the port, not a match)
 * Update of the swinging laser (class 123, levels 15 and 17). The emitter moves about its placed
 * point (pvars +0x00) by its kind (+0x20): 0 circles at radius r (+0x24) in the vertical plane across
 * its yaw (+0x2C), 1-2 slide r*cos t along its heading or up and down, 3-4 swing as a pendulum
 * (4 a quarter turned); t (+0x28) advances at the speed (+0x30) along the path and the animation
 * speed follows. Every frame its beam runs 12 along the moby's z row: a hit line (damage 1,
 * 0x10001) hits what it crosses, a world line ends it (+0x10 = the end), D_L15_001615D8 sparks run
 * along it, and the draw callback func_L15_002A8850 puts the beam quad on it.
 * Adapted with ReRAC's swing_laser.rs (ISC), which documents the US twin (level15 0x2a6e68);
 * shaped after an earlier worker's attempt (build-sn/try/func_L15_002A8090/p2.c).
 * equiv: DIFFERENT by shape only: in state 0 the kind-0 test has the other branch polarity and the
 * constant 3 for the state is set before a label in retail; this rebuilds m + 0xC0 and the frame
 * address +0x40 once more than retail. */
typedef unsigned int Q_2A8090 __attribute__((mode(TI), aligned(16)));
#define FB_2A8090 u_2A8090.c
#define FQ_2A8090(o) (*(Q_2A8090 *)(FB_2A8090 + (o)))
#define FF_2A8090(o) (*(float *)(FB_2A8090 + (o)))
#define DF_2A8090(o) (*(float *)(d + (o)))

extern Q_2A8090 D_L15_001744E0_2A8090 __asm__("D_L15_001744E0");
extern float D_0015EE6C_2A8090 __asm__("D_0015EE6C") MACRO_ADDR;
extern int D_L15_001615D8_2A8090 SDATA(D_L15_001615D8);
extern float D_L15_001615DC_2A8090 SDATA(D_L15_001615DC);
extern float D_L15_001615E0_2A8090 SDATA(D_L15_001615E0);

extern float func_001FA748_2A8090(float, float) __asm__("func_001FA748");
extern float func_001F9F90_2A8090(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2A8090(float) __asm__("func_001F9FA8");
extern void func_002156E0_2A8090(void *, void *, void *, float) __asm__("func_002156E0");
extern void func_001F9BD8_2A8090(void *, void *, void *) __asm__("func_001F9BD8");
extern void func_001FA218_2A8090(void *, void *) __asm__("func_001FA218");
extern void func_001FA480_2A8090(void *, void *) __asm__("func_001FA480");
extern void func_00214F78_2A8090(void *) __asm__("func_00214F78");
extern float func_001FA7D8_2A8090(float) __asm__("func_001FA7D8");
extern float func_001FA790_2A8090(float, float) __asm__("func_001FA790");
extern void func_00215C00_2A8090(void *, float, float, float) __asm__("func_00215C00");
extern void func_001F9EC0_2A8090(void *, void *, void *) __asm__("func_001F9EC0");
extern void func_L00_001FF4B0_2A8090(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void func_001F9BF0_2A8090(void *, void *, void *) __asm__("func_001F9BF0");
extern int func_L00_001EFFF0_2A8090(void *, void *, int, void *, void *) __asm__("func_L00_001EFFF0");
extern int func_001160D8_2A8090(void) __asm__("func_001160D8");
extern float func_002140F8_2A8090(float, float) __asm__("func_002140F8");
extern float func_001F9878_2A8090(float) __asm__("func_001F9878");
extern int func_001FA898_2A8090(float) __asm__("func_001FA898");
extern void func_001F9C30_2A8090(void *, void *, float) __asm__("func_001F9C30");
extern float func_001F9CB8_2A8090(void *) __asm__("func_001F9CB8");
extern void func_L00_001FF240_2A8090(void *, void *, void *) __asm__("func_L00_001FF240");
extern float func_001FA888_2A8090(int) __asm__("func_001FA888");
extern int func_002140B0_2A8090(int) __asm__("func_002140B0");
extern void func_L00_00273F80_2A8090(void *, void *, int, int, int, int, float) __asm__("func_L00_00273F80");
extern void func_001F49B0_2A8090(void (*)(void), void *) __asm__("func_001F49B0");
extern void func_L15_002A8850_2A8090(void) __asm__("func_L15_002A8850");

void func_L15_002A8090(unsigned char *m) {
    union {
        Q_2A8090 q[10];
        char c[0xA0];
    } u_2A8090;
    char *d;
    float rate, a0, a1, s, len, sc;
    int k, life, life8;

    FQ_2A8090(0x00) = *(Q_2A8090 *)(m + 0x10);
    d = *(char **)(m + 0x78);
    switch (m[0x20]) {
    case 0:
        *(Q_2A8090 *)d = FQ_2A8090(0x00);
        *(float *)(m + 0x58) = DF_2A8090(0x30) / 1.5f;
        DF_2A8090(0x28) = DF_2A8090(0x28) * 0.017453292f;
        if (*(int *)(d + 0x20) == 0) {
            DF_2A8090(0x2C) = *(float *)(m + 0x48);
            m[0x20] = 1;
        } else if (*(int *)(d + 0x20) < 3) {
            m[0x20] = 3;
        } else {
            m[0x20] = 2;
            DF_2A8090(0x2C) = *(float *)(m + 0x48);
        }
        break;
    case 1:
        FQ_2A8090(0x30) = 0;
        FQ_2A8090(0x40) = 0;
        FF_2A8090(0x38) = DF_2A8090(0x24);
        FQ_2A8090(0x20) = FQ_2A8090(0x30);
        rate = 360.0f / (DF_2A8090(0x24) * 6.2831855f / DF_2A8090(0x30)) * 0.017453292f * D_0015EE6C_2A8090;
        DF_2A8090(0x28) = func_001FA748_2A8090(DF_2A8090(0x28), rate);
        *(float *)(m + 0x58) = DF_2A8090(0x30) / 1.5f;
        FF_2A8090(0x30) = func_001F9F90_2A8090(DF_2A8090(0x2C));
        FF_2A8090(0x34) = func_001F9FA8_2A8090(DF_2A8090(0x2C));
        *(int *)(FB_2A8090 + 0x38) = 0;
        func_002156E0_2A8090(FB_2A8090 + 0x10, FB_2A8090 + 0x20, FB_2A8090 + 0x30, DF_2A8090(0x28));
        func_001F9BD8_2A8090(FB_2A8090 + 0x90, d, FB_2A8090 + 0x10);
        *(Q_2A8090 *)(m + 0x10) = FQ_2A8090(0x90);
        FF_2A8090(0x48) = func_001FA748_2A8090(DF_2A8090(0x2C), 1.5707964f);
        FF_2A8090(0x44) = func_001FA748_2A8090(-DF_2A8090(0x28), 3.1415927f);
        func_001FA218_2A8090(FB_2A8090 + 0x50, FB_2A8090 + 0x40);
        func_001FA480_2A8090(m + 0xC0, FB_2A8090 + 0x50);
        func_00214F78_2A8090(m + 0xC0);
        *(unsigned short *)(m + 0x34) |= 0x100;
        break;
    case 2:
        FQ_2A8090(0x30) = 0;
        FQ_2A8090(0x40) = 0;
        FF_2A8090(0x38) = -DF_2A8090(0x24);
        FQ_2A8090(0x20) = FQ_2A8090(0x30);
        a0 = func_001FA7D8_2A8090(func_001F9F90_2A8090(DF_2A8090(0x28)) * 1.5707964f);
        a0 = func_001FA748_2A8090(a0, *(int *)(d + 0x20) != 4 ? 0.0f : 1.5707964f);
        rate = 360.0f / (DF_2A8090(0x24) * 6.2831855f / DF_2A8090(0x30)) * 0.017453292f * D_0015EE6C_2A8090;
        s = func_001FA748_2A8090(DF_2A8090(0x28), rate);
        DF_2A8090(0x28) = s;
        a1 = func_001FA7D8_2A8090(func_001F9F90_2A8090(s) * 1.5707964f);
        a1 = func_001FA748_2A8090(a1, *(int *)(d + 0x20) != 4 ? 0.0f : 1.5707964f);
        s = func_001FA790_2A8090(a1, a0);
        *(float *)(m + 0x58) = DF_2A8090(0x24) * s / (D_0015EE6C_2A8090 * 1.5f);
        FF_2A8090(0x30) = func_001F9F90_2A8090(DF_2A8090(0x2C));
        FF_2A8090(0x34) = func_001F9FA8_2A8090(DF_2A8090(0x2C));
        *(int *)(FB_2A8090 + 0x38) = 0;
        func_002156E0_2A8090(FB_2A8090 + 0x10, FB_2A8090 + 0x20, FB_2A8090 + 0x30, a1);
        func_001F9BD8_2A8090(FB_2A8090 + 0x90, d, FB_2A8090 + 0x10);
        *(Q_2A8090 *)(m + 0x10) = FQ_2A8090(0x90);
        FF_2A8090(0x48) = func_001FA748_2A8090(DF_2A8090(0x2C), 1.5707964f);
        FF_2A8090(0x44) = func_001FA748_2A8090(-a1, 0.0f);
        func_001FA218_2A8090(FB_2A8090 + 0x50, FB_2A8090 + 0x40);
        func_001FA480_2A8090(m + 0xC0, FB_2A8090 + 0x50);
        func_00214F78_2A8090(m + 0xC0);
        *(unsigned short *)(m + 0x34) |= 0x100;
        break;
    case 3:
        a0 = func_001F9F90_2A8090(DF_2A8090(0x28)) * DF_2A8090(0x24);
        rate = 360.0f / (DF_2A8090(0x24) * 6.2831855f / DF_2A8090(0x30)) * 0.017453292f * D_0015EE6C_2A8090;
        s = func_001FA748_2A8090(DF_2A8090(0x28), rate);
        DF_2A8090(0x28) = s;
        a1 = func_001F9F90_2A8090(s) * DF_2A8090(0x24);
        *(float *)(m + 0x58) = (a1 - a0) / (D_0015EE6C_2A8090 * 1.5f);
        func_00215C00_2A8090(FB_2A8090 + 0x10, a1, *(int *)(d + 0x20) == 2 ? 0.0f : *(float *)(m + 0x48),
                             *(int *)(d + 0x20) == 2 ? 1.5707964f : 0.0f);
        func_001F9BD8_2A8090(FB_2A8090 + 0x20, d, FB_2A8090 + 0x10);
        *(Q_2A8090 *)(m + 0x10) = FQ_2A8090(0x20);
        break;
    }

    /* the beam: 12 along the z row, a hit line, a world line, the sparks */
    FQ_2A8090(0x20) = 0;
    FF_2A8090(0x28) = 12.0f;
    func_001F9EC0_2A8090(FB_2A8090 + 0x20, FB_2A8090 + 0x20, m + 0xC0);
    func_001F9BD8_2A8090(FB_2A8090 + 0x40, m + 0x10, FB_2A8090 + 0x20);
    FQ_2A8090(0x10) = FQ_2A8090(0x40);
    *(int *)(FB_2A8090 + 0x54) = 0x10001;
    *(unsigned char **)(FB_2A8090 + 0x50) = m;
    FF_2A8090(0x5C) = 1.0f;
    *(int *)(FB_2A8090 + 0x60) = 1;
    func_L00_001FF4B0_2A8090(FB_2A8090 + 0x40, FB_2A8090 + 0x20, 1.0f);
    *(unsigned short *)(FB_2A8090 + 0x5A) = *(unsigned short *)(m + 0xA6);
    FF_2A8090(0x48) = 1.0f;
    FF_2A8090(0x4C) = 5627.9248f;
    FB_2A8090[0x59] = 1;
    FB_2A8090[0x58] = 1;
    func_001F9BF0_2A8090(FB_2A8090 + 0x70, m + 0x10, FB_2A8090);
    FQ_2A8090(0x30) = FQ_2A8090(0x70);
    func_L00_001EFFF0_2A8090(m + 0x10, FB_2A8090 + 0x10, 9, m, FB_2A8090 + 0x40);
    if (func_L00_001EFFF0_2A8090(m + 0x10, FB_2A8090 + 0x10, 2, 0, 0) != 0) {
        *(Q_2A8090 *)(d + 0x10) = D_L15_001744E0_2A8090;
    } else {
        *(Q_2A8090 *)(d + 0x10) = FQ_2A8090(0x10);
    }
    for (k = 0; k < D_L15_001615D8_2A8090; ) {
        sc = (func_001160D8_2A8090() & 1) ? 1.0f : -1.0f;
        life = func_001FA898_2A8090(
            func_001F9878_2A8090(func_002140F8_2A8090(D_L15_001615E0_2A8090 * 0.5f, D_L15_001615E0_2A8090)));
        func_001F9BF0_2A8090(FB_2A8090 + 0x70, d + 0x10, m + 0x10);
        FQ_2A8090(0x20) = FQ_2A8090(0x70);
        func_001F9C30_2A8090(FB_2A8090 + 0x20, FB_2A8090 + 0x20, sc);
        len = func_001F9CB8_2A8090(FB_2A8090 + 0x20);
        if (sc < 0.0f) {
            FQ_2A8090(0x10) = *(Q_2A8090 *)(d + 0x10);
        } else {
            FQ_2A8090(0x10) = *(Q_2A8090 *)(m + 0x10);
        }
        life8 = life & 0xFF;
        k++;
        func_001F9C30_2A8090(FB_2A8090 + 0x20, FB_2A8090 + 0x20, func_002140F8_2A8090(0.0f, 0.9f));
        func_L00_001FF240_2A8090(FB_2A8090 + 0x70, FB_2A8090 + 0x10, FB_2A8090 + 0x20);
        s = D_0015EE6C_2A8090 * 10.0f;
        func_L00_001FF4B0_2A8090(FB_2A8090 + 0x20, FB_2A8090 + 0x20,
                                 func_002140F8_2A8090(s, len / func_001FA888_2A8090(life << 2)));
        func_L00_00273F80_2A8090(FB_2A8090 + 0x10, FB_2A8090 + 0x20, 0x604040FF, life8, func_002140B0_2A8090(0xFF) & 0xFF, 0,
                                 D_L15_001615DC_2A8090);
    }
    func_001F49B0_2A8090(func_L15_002A8850_2A8090, m);
}
#undef FB_2A8090
#undef FQ_2A8090
#undef FF_2A8090
#undef DF_2A8090
