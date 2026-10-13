/* functional: high. Same reads, writes and calls with the same arguments in the same order as retail; only registers, addressing and scheduling differ, plus both stack vectors are zeroed on entry because retail's first hero spark normalises whatever stack residue sp+0x00 holds (zero normalises to zero in func_L00_001FF4B0) and neither w is ever set. */
/* func_L10_002BFBB8 -- src/overlays/l10_orxon/vendor_00296BD8.c (functional C for the port, not a match)
 * Level 10 (Orxon) update of a moby (pvars +0x78, first word = index of a linked moby in D_L10_00160058):
 * on its first frame (+0x20 == 0) it clears the countdown D_L10_00161928 and converts the two durations
 * D_L10_00161924 / D_L10_00161920 with scale_ticks (func_001F9850). While D_0013E633[0x2EC1] is 1, the pvars
 * name a moby and the hero (D_0013E633+0xE9D) is within 0.5 (xy) of this moby, it bursts 10 sparks around the
 * hero (func_L00_00272F00), calls func_0022ED80(0, 0, m), func_L00_00217718 on the linked moby (0x43, 0),
 * starts the countdown from D_L10_00161920, resets a camera/object field at D_L10_00167180+0x180 (timer 90 ticks,
 * copies its +0x00 quad to +0x40) and bursts 10 more sparks around the linked moby. Every scale_ticks(60)
 * frames (D_L10_0015F6B0) it spawns func_001FA898(16.0) puffs (func_L00_00272488) rising above itself. It then
 * sets its colour (+0x90) to a 150-frame sine blend of 0x80464646 and 0x80828282 (FastTweenColor), ticks the
 * countdown (func_001F9908) and, while it runs, drives the shake/effect object of D_L00_00166F00
 * (func_L00_002E9A40(0.002 + 0.008 * remaining / total, 0.2), 2E9B30, 2E9AF8, 2E9A18(0)).
 * func_L00_001FF240 (adds the third vector's xyz into the second, in place; the first is unused) and func_L00_001FF4B0 (normalise and scale) are VU0
 * helpers called as functions, as in retail; this function itself has no COP2 code. */
typedef unsigned int Q_2BFBB8 __attribute__((mode(TI), aligned(16)));

extern char D_0013E633_2BFBB8[] __asm__("D_0013E633");
extern char *D_L10_00160058_2BFBB8 __asm__("D_L10_00160058") MACRO_ADDR;
extern char D_L10_00167180_2BFBB8[] __asm__("D_L10_00167180");
extern int D_L10_0015F6B0_2BFBB8 __asm__("D_L10_0015F6B0") MACRO_ADDR;
extern int D_L10_00161920_2BFBB8 __asm__("D_L10_00161920") MACRO_ADDR;
extern int D_L10_00161924_2BFBB8 __asm__("D_L10_00161924") MACRO_ADDR;
extern int D_L10_00161928_2BFBB8 __asm__("D_L10_00161928") MACRO_ADDR;
extern float D_0015EE6C_2BFBB8 __asm__("D_0015EE6C") MACRO_ADDR;
extern float D_0015EE70_2BFBB8 __asm__("D_0015EE70") MACRO_ADDR;

extern int func_001F9850_2BFBB8(int) __asm__("func_001F9850");
extern float func_001F9D48_2BFBB8(void *, void *) __asm__("func_001F9D48");
extern float func_002140F8_2BFBB8(float, float) __asm__("func_002140F8");
extern float func_00214158_2BFBB8(void) __asm__("func_00214158");
extern int func_002140B0_2BFBB8(int) __asm__("func_002140B0");
extern int func_L00_00258BC8_2BFBB8(int, int) __asm__("func_L00_00258BC8");
extern float func_001F9F90_2BFBB8(float) __asm__("func_001F9F90");
extern float func_001F9FA8_2BFBB8(float) __asm__("func_001F9FA8");
extern void func_L00_001FF240_2BFBB8(void *, void *, void *) __asm__("func_L00_001FF240");
extern void func_L00_001FF4B0_2BFBB8(void *, void *, float) __asm__("func_L00_001FF4B0");
extern void *func_L00_00272F00_2BFBB8(void *, int, float, float, int, int, int, void *, float) __asm__("func_L00_00272F00");
extern void *func_L00_00272488_2BFBB8(void *, int, int, float, float) __asm__("func_L00_00272488");
extern int func_0022ED80_2BFBB8(int, int, int) __asm__("func_0022ED80");
extern void func_L00_00217718_2BFBB8(void *, void *, int, int) __asm__("func_L00_00217718");
extern float func_001FA888_2BFBB8(int) __asm__("func_001FA888");
extern int func_001FA898_2BFBB8(float) __asm__("func_001FA898");
extern unsigned int func_001FA8A8_2BFBB8(unsigned int, unsigned int, float) __asm__("func_001FA8A8");
extern int func_001F9908_2BFBB8(int *) __asm__("func_001F9908");
extern void func_L00_002E9A40_2BFBB8(float, float) __asm__("func_L00_002E9A40");
extern void func_L00_002E9B30_2BFBB8(void) __asm__("func_L00_002E9B30");
extern void func_L00_002E9AF8_2BFBB8(void) __asm__("func_L00_002E9AF8");
extern void func_L00_002E9A18_2BFBB8(int) __asm__("func_L00_002E9A18");

/* One spark of a burst: pos (v10) gets a random xy offset (radius up to 0.5) and z in [0, 0.5) added to
 * the centre; vel (v0) is normalised to a random length up to D_0015EE6C, then gets an upward z. */
static inline __attribute__((always_inline)) void spark_2BFBB8(float *v10, float *v0, float *v20, float f21, int r, int g, int b, int *d)
{
    float t;
    int life, col, dir;
    char *centre;

    t = func_001F9F90_2BFBB8(func_00214158_2BFBB8());
    v10[0] = t * func_002140F8_2BFBB8(0.0f, 0.5f);
    t = func_001F9FA8_2BFBB8(func_00214158_2BFBB8());
    v10[1] = t * func_002140F8_2BFBB8(0.0f, 0.5f);
    v10[2] = 0.0f;
    v10[2] = v10[2] + func_002140F8_2BFBB8(0.0f, 0.5f);
    /* centre: the hero (d == 0) or the linked moby, read here as retail does */
    if (d == 0) {
        centre = D_0013E633_2BFBB8 + 0xE9D;
    } else {
        centre = D_L10_00160058_2BFBB8 + (*d << 8) + 0x10;
    }
    func_L00_001FF240_2BFBB8(v20, v10, centre);
    func_L00_001FF4B0_2BFBB8(v0, v0, func_002140F8_2BFBB8(0.0f, D_0015EE6C_2BFBB8));
    v0[2] = func_002140F8_2BFBB8(D_0015EE6C_2BFBB8 * 2.5f, D_0015EE6C_2BFBB8 * 5.0f);
    life = func_001F9850_2BFBB8(func_L00_00258BC8_2BFBB8(0xF, 0x1E));
    col = (b << 16) | ((g << 8) | 0x7F000000) | r;
    dir = func_002140B0_2BFBB8(2) ? 1 : -1;
    func_L00_00272F00_2BFBB8(v10, life, f21 * 0.07f, f21 * 0.7f, col, 0, dir, v0, D_0015EE70_2BFBB8 * 15.0f);
}

void func_L10_002BFBB8(char *m)
{
    int *d = *(int **)(m + 0x78);
    float v0[4] __attribute__((aligned(16))); /* retail sp+0x00: unset before the first burst */
    float v10[4] __attribute__((aligned(16)));  /* retail sp+0x10: w never set */
    float v20[4] __attribute__((aligned(16)));  /* retail sp+0x20: passed to func_L00_001FF240, which ignores it */
    int i, r, g, b, n;
    float f21, a, f;

    /* host only: retail leaves both vectors as stack residue until written */
    *(Q_2BFBB8 *)v0 = 0;
    *(Q_2BFBB8 *)v10 = 0;

    if (*(unsigned char *)(m + 0x20) == 0) {
        int x = D_L10_00161924_2BFBB8;
        D_L10_00161928_2BFBB8 = 0;
        D_L10_00161924_2BFBB8 = func_001F9850_2BFBB8(x);
        D_L10_00161920_2BFBB8 = func_001F9850_2BFBB8(D_L10_00161920_2BFBB8);
        *(unsigned char *)(m + 0x20) = 1;
    }

    if ((unsigned char)D_0013E633_2BFBB8[0xE1D + 0x20A4] == 1 && *d != -1 &&
        func_001F9D48_2BFBB8(m + 0x10, D_0013E633_2BFBB8 + 0xE9D) < 0.5f) {
        char *p;

        /* burst around the hero */
        for (i = 9; i >= 0; i--) {
            f21 = func_002140F8_2BFBB8(0.1f, 1.0f);
            r = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            g = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            b = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            spark_2BFBB8(v10, v0, v20, f21, r, g, b, 0);
        }

        func_0022ED80_2BFBB8(0, 0, (int)m);
        p = D_L10_00160058_2BFBB8 + (*d << 8);
        func_L00_00217718_2BFBB8(p + 0x10, p + 0x40, 0x43, 0);
        D_L10_00161928_2BFBB8 = D_L10_00161920_2BFBB8;
        p = *(char **)(D_L10_00167180_2BFBB8 + 0x180);
        if (*(short *)(p + 0x86) == 0) {
            char *q = *(char **)(p + 0x70);
            *(short *)(q + 0x20) = func_001F9850_2BFBB8(0x5A);
            *(Q_2BFBB8 *)(p + 0x40) = *(Q_2BFBB8 *)(p + 0x00);
        }

        /* burst around the linked moby */
        for (i = 9; i >= 0; i--) {
            *(Q_2BFBB8 *)v10 = 0;
            v10[0] = func_002140F8_2BFBB8(-1.0f, 1.0f);
            v10[1] = func_002140F8_2BFBB8(-1.0f, 1.0f);
            *(Q_2BFBB8 *)v0 = *(Q_2BFBB8 *)v10;
            f21 = func_002140F8_2BFBB8(0.1f, 1.0f);
            r = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            g = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            b = func_L00_00258BC8_2BFBB8(0x40, 0xFF);
            spark_2BFBB8(v10, v0, v20, f21, r, g, b, d);
        }
    }

    n = func_001F9850_2BFBB8(0x3C);
    if (D_L10_0015F6B0_2BFBB8 % n == 0 && *d != -1) {
        for (i = 0; i < func_001FA898_2BFBB8(16.0f); i++) {
            char *s;
            *(Q_2BFBB8 *)v0 = *(Q_2BFBB8 *)(m + 0x10);
            f = func_001FA888_2BFBB8(i) * 0.1f * 0.0625f + 1.8f;
            v0[2] = v0[2] + f;
            s = func_L00_00272488_2BFBB8(v0, 0, 0x1EFF7F40, 0.0f, D_0015EE6C_2BFBB8 * 315000.0f);
            if (s != 0) {
                *(int *)(s + 0x20) = 0;
                *(int *)(s + 0x24) = 0;
                *(float *)(s + 0x28) = D_0015EE6C_2BFBB8 * -1.5f;
            }
        }
    }

    a = (float)(D_L10_0015F6B0_2BFBB8 % 150) / 150.0f;
    a = (a + a) * 3.1415927f;
    if (3.1415927f < a) {
        a = a - 6.2831855f;
    }
    *(unsigned int *)(m + 0x90) =
        func_001FA8A8_2BFBB8(0x80464646, 0x80828282, (func_001F9FA8_2BFBB8(a) + 1.0f) * 0.5f);

    func_001F9908_2BFBB8(&D_L10_00161928_2BFBB8);
    n = D_L10_00161928_2BFBB8;
    if (n != 0) {
        if (D_L10_00161924_2BFBB8 < n) {
            f = 0.002f;
        } else {
            f = func_001FA888_2BFBB8(n) / func_001FA888_2BFBB8(D_L10_00161924_2BFBB8) * 0.007999999f + 0.002f;
        }
        func_L00_002E9A40_2BFBB8(f, 0.2f);
        func_L00_002E9B30_2BFBB8();
        func_L00_002E9AF8_2BFBB8();
        func_L00_002E9A18_2BFBB8(0);
    }
}
