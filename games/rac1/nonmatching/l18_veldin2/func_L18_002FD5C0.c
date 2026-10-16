/* NON_MATCHING func_L18_002FD5C0 -- src/overlays/l18_veldin2/vendor_002F9D48.c
 * Best so far: BYTES 36/988 (96.4% of the bytes match), checked 2026-10-03.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   `$a2 = p + 0x88` and `$a3 = p + 0x8C` before the call, and the callee's own
 *   prologue copies `$6` and `$7` into saved registers. (`asm/overlays/
 *   func_L00_0025BBA0.s` shows `daddu $18,$7,$0` / `daddu $17,$6,$0`.)
 *   - The four floats at p+0x80..0x8C are stored **off p with an offset each**, not
 *   through a `float *u = (float *)(p + 0x80)` pointer: the pointer costs the
 *   `addiu $v0,$s1,0x80` that made us a word too long.
 *   Remaining 36 bytes: register picks in the tail (retail keeps the list base in
 *   $a0/$v1 where we use $a1/$v0) and one `bne` where retail has `bnel`.
 */
typedef struct { float v[4]; } __attribute__((aligned(16))) QV;
extern short D_EE70_s __asm__("D_0015EE70");
extern float func_001FA748(float, float);
extern float func_001F9FA8(float);
extern int func_001FA8A8(int, int, float);
extern int func_001F9850(int);
extern void func_00213DE0(void *, int, int, int);
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, void *, void *, int, int);
extern int func_002140B0(int);
extern void func_L00_0025BBA0(void *, void *, void *, void *);
extern void func_L00_0025D5B0(float ang, char *o, float *s, int a, int b, int c);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_00260FB0(float a, char *m, char *b, int c, int d, int *e, int f);
extern void func_DF58_v(void) __asm__("func_L18_002FDF58");
extern float D_0015EE6C MACRO_ADDR;
extern int *D_L18_001B11B0[];
extern char D_0013E633[];
extern int D_L18_0015F6B0_g __asm__("D_L18_0015F6B0") MACRO_ADDR;
extern int D_L18_0015F6A8 MACRO_ADDR;
extern short D_L18_00162694;
extern short D_L18_0016269C;
extern short D_L18_001626A0;
extern float D_L18_001626A4 MACRO_ADDR;
extern short D_L18_001626A8;
extern short D_L18_0016268C;
extern short D_L18_001626AC;
extern short D_L18_001626B0;
extern short D_L18_001626B4;
extern short D_L18_001626B8;

void func_L18_002FD5C0(unsigned char *m) {
    char *p = *(char **)(m + 0x78);
    int *e;
    char *g1;
    QV v;
    int i;
    float f;
    float ang;
    char *r;
    if (m[0x20] == 0 || m[0x20] == 8) return;
    if (m[0x20] != 7 && D_L18_0015F6B0_g != (*(int *)&D_L18_00162694)) {
        (*(int *)&D_L18_00162694) = D_L18_0015F6B0_g;
        func_001F49B0(func_DF58_v, m);
    }
    {
        float s = D_L18_001626A4;
        if ((unsigned char)(m[0x20] - 4) < 2) s = *(float *)&D_L18_001626A8;
        *(float *)(p + 0x1F8) = func_001FA748(*(float *)(p + 0x1F8), s * 0.01745329f * D_0015EE6C);
    }
    *(int *)(m + 0x90) = func_001FA8A8((*(int *)&D_L18_0016269C), (*(int *)&D_L18_001626A0), func_001F9FA8(*(float *)(p + 0x1F8)) * 0.5f + 0.5f);
    if (*(float *)(m + 0x18) < 100.0f && m[0x20] != 7) m[0x20] = 6;
    *(float *)(m + 0x2C) = *(float *)(*(char **)(m + 0x24) + 0x24) * *(float *)&D_L18_0016268C;
    g1 = D_0013E633 + 0xE1D;
    if (*(int *)(g1 + 0x2084) == 0x72 || D_L18_0015F6A8 == 2) {
        if (m[0x20] != 7) {
            m[0x20] = 8;
            if (m[0x53]) func_00213DE0(m, 0, 0, func_001F9850(10));
            *(int *)(m + 0x94) = 0;
            *(unsigned short *)(m + 0x34) = (*(unsigned short *)(m + 0x34) | 0x41) & 0xEFFF;
            return;
        }
    }
    {
        char *q;
        f = 0.0f;
        q = func_L00_0025B478(m, 0x330000, 0);
        func_L00_0025B4D0(m, q, p + 0x20, 0, &i, &f, 0, 4);
        if (i != 1 && m[0x20] != 6) {
            if (f != 0.0f) {
                float r = *(float *)(p + 0x20) - f;
                *(float *)(p + 0x20) = r;
                if (r <= 0.0f) {
                    if (func_002140B0(2)) {
                        m[0x20] = 6;
                    } else {
                        float k = *(float *)&D_EE70_s;
                        float a = (*(float *)&D_L18_001626AC) * k;
                        float b = (*(float *)&D_L18_001626B0) * k;
                        float c = (*(float *)&D_L18_001626B4) * D_0015EE6C;
                        float d = (*(float *)&D_L18_001626B8) * D_0015EE6C;
                        p[0xAD] = 0;
                        *(float *)(p + 0xBC) = D_0015EE6C + D_0015EE6C;
                        *(float *)(p + 0x8C) = d;
                        *(float *)(p + 0x80) = a;
                        *(float *)(p + 0x84) = b;
                        *(float *)(p + 0x88) = c;
                        *(int *)(p + 0x94) = 0x29;
                        *(int *)(p + 0x90) = 0x200;
                        *(float *)(p + 0x98) = 0.5f;
                        *(unsigned short *)(m + 0x34) &= 0xEFFF;
                        *(QV *)&v = *(QV *)(q + 0x10);
                        func_L00_0025BBA0(&v, &ang, p + 0x88, p + 0x8C);
                        func_L00_0025D5B0(ang, (char *)m, (float *)(p + 0x70), 3, 1, 0);
                        *(float *)(p + 0xC0) = 7.5f;
                        *(float *)(p + 0xC4) = 15.0f;
                        m[0x20] = 5;
                        *(int *)(m + 0x94) = 0;
                        p[0x67] = 0x78;
                        func_L00_0025E4B0(m, (short *)(p + 0x60));
                    }
                }
            }
        }
    }
    r = p + 0x180;
    m[0xA4] = 0xFF;
    func_L00_0025E590(m, p + 0x60);
    e = D_L18_001B11B0[*(int *)(p + 0x1E0)];
    func_L00_00260FB0(512.0f, m, r, 0, 0, e + 4, *e);
    if (*(int *)(p + 0x1C4) == 2) {
        char *g = D_0013E633 + 0xE1D;
        char *s = g + 0x80;
        if (*(int *)(g + 0x208C) == 0xF) {
            int *t = *(int **)(g + 0x560);
            s = (char *)t + t[0] * 16;
        }
        qcopy(r, s);
    }
    if (*(int *)(p + 0x1C0) == 0) {
        char *g3 = D_0013E633 + 0xE1D;
        *(int *)(p + 0x1C0) = *(int *)(g3 + 0x2080);
    }
}
