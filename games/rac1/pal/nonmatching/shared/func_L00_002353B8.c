/* NON_MATCHING func_L00_002353B8 -- src/overlays/shared/help_00232560.c
 * Best so far: SIZE ours 604 / retail 588, checked 2026-10-12.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Hero camera-shake/glow setup: clamps a scale in f20, scales attached objects' float at +0x2C (types 0x1B1/0x50
 *   p5.c is same size as retail (26 bytes differ): retail copies the loop base register (`daddu $6,$3,$0` then `ad
 *   Tried: separate base locals, do-while with explicit pre-test, second constant load for the loop base, pointer-
 *   Idioms found: `extern int D_0015EE84 MACRO_ADDR;` alone gives retail's gp delay-slot load + lui reload; func_L
 *   y03 (fz5): hero glow setup. p9-p12 (pp from global, do-while with explicit guard, count read via global, block
 *   hq12 s06 (5 runs, stopped): best.c did not compile: its `extern float D_0015EE6C MACRO_ADDR;` and `void func_0
 */
extern short D_0015EF14;
extern int D_0015EE84 MACRO_ADDR;
extern char D_L00_0017C440[];
extern char D_L00_0016C960[];
extern char D_0013E633[] NOT_SDA;
extern float func_001FA748(float, float);
extern int func_001FA8A8(int, int, float);

// Sets up the hero's camera-shake/glow object: clamps a scale, scales attached objects and sets a colour from a wobble.
void func_L00_002353B8(char *obj) {
    float f20 = *(float *)&D_0015EF14;
    char *s0;
    char *q;
    char *q2;
    char *b;
    int c1, c0;
    float t;
    if (D_L00_0015F6A8 == 6) {
        if (1.33f < f20) f20 = 1.33f;
    }
    s0 = D_L00_0017C440;
    if (*(unsigned char *)(s0 + 1) == 0) {
        func_0020D960(obj, 0x18, s0);
        *(float *)(s0 + 0x20) = f20;
        *(float *)(s0 + 0x24) = f20;
        *(float *)(s0 + 0x28) = f20;
    }
    if (D_0015EE84 == 2 || D_0015EE84 == 9 || D_0015EE84 == 0xB || D_0015EE84 == 0xD) {
        int i;
        char *b1 = D_L00_0016C960;
        char **pp = (char **)(b1 + 0x178);
        for (i = 0; i < *(short *)(b1 + 0x44); i++) {
            char *m = pp[i];
            if (m != 0) {
                int h = *(short *)(m + 0xA6);
                if (h == 0x1B1 || h == 0x50A || h == 0x509) {
                    *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * f20;
                }
            }
        }
    }
    q = D_0013E633 + 0xE1D;
    c1 = 0x806E8C6E;
    c0 = 0x805A3232;
    *(float *)(q + 0x1628) = func_001FA748(*(float *)(q + 0x1628), D_0015EE6C * 2.0943951f);
    if (D_0015EE84 == 0) {
        b = D_L00_0016C960;
        if (*(int *)(b + 0x30) == 0) {
            if (*(int *)(b + 0x34) >= func_001F9850(0x343)) {
                if (*(int *)(b + 0x34) <= func_001F9850(0x438)) {
                    c1 = 0x806EAAFA;
                    c0 = 0x80326E5A;
                    *(float *)(q + 0x1628) = func_001FA748(*(float *)(q + 0x1628), D_0015EE6C * 5.9341192245f);
                }
            }
        }
    }
    q2 = D_0013E633 + 0xE1D;
    t = func_001F9FA8(*(float *)(q2 + 0x1628));
    *(int *)(obj + 0x90) = func_001FA8A8(c1, c0, t * 0.5f + 0.5f);
    func_L00_002352D0((int)obj);
}
