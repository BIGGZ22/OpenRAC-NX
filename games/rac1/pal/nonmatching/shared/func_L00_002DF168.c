/* NON_MATCHING func_L00_002DF168 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 664 / retail 680, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Camera matrix setup: picks 3 vec4 from the hero/camera by state, builds matrices via func_L00_001FF4B0, fills 
 *   Left: retail keeps the &s[3] pointer in $s0 plus a copy in $s4 for the loop (daddu $s4,$s0) and loop addresses
 */
typedef int u128_DF168 __attribute__((mode(TI)));
extern void func_001F9C30(void *, void *, float);
extern void func_L00_001FF4B0(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);
extern void func_001F9BC0(void *);
extern float func_00214158(void);
extern void func_002156E0(void *, void *, float, void *);
extern float D_L00_00161C54 MACRO_ADDR;
extern char D_L00_001670D0[];
extern char D_L00_001E6290[];
extern short D_L00_001E6358[];
extern char D_L00_001E6280[];
extern char D_L00_00161C30[];
extern char D_L00_00161C40[];
extern float D_L00_00161C50 MACRO_ADDR;
extern char D_L00_001E4FD0_x[] __asm__("D_L00_001E4FD0");
extern short D_L00_00161C10;
extern short D_L00_00161C14;
extern short D_L00_00161C20;
extern int D_L00_00161C24;
extern int D_L00_00161C28;

// Builds the camera matrices and resets the camera effect state.
void func_L00_002DF168(char *a, char *b) {
    u128_DF168 s[7];
    char *g, *v, *h, *p, *q;
    short *w;
    int i, st;
    if (*(int *)&D_L00_00161C10 == 0) return;
    g = D_0013E633 + 0xE1D;
    D_L00_00161C54 = 1.7f;
    *(int *)&D_L00_00161C10 = 0;
    st = *(int *)(g + 0x2084);
    if ((st == 0x1E || st == 1) && (*(unsigned short *)(*(char **)(g + 0x2080) + 0x34) & 1)) {
        qcopy((char *)s, D_L00_001670D0);
        qcopy((char *)s + 0x10, D_L00_001670D0 + 0x10);
        v = (char *)s + 0x20;
        qcopy(v, D_L00_001670D0 + 0x20);
    } else {
        g = D_0013E633 + 0xE1D;
        if (*(short *)(g + 0x22C8) == 1) {
            func_001F9C30(s, a + 0xD0, -1.0f);
            qcopy((char *)s + 0x10, a + 0xC0);
            v = (char *)s + 0x20;
            qcopy(v, a + 0xE0);
        } else {
            qcopy((char *)s, g + 0x640);
            qcopy((char *)s + 0x10, g + 0x650);
            v = (char *)s + 0x20;
            qcopy(v, g + 0x660);
        }
    }
    h = (char *)s + 0x30;
    func_L00_001FF4B0(h, s, D_L00_00161C54);
    func_L00_001FF4B0((char *)s + 0x40, v, 1.0f);
    qcopy(D_L00_001E6290, b);
    D_L00_001E6358[0] = 0;
    p = D_L00_001E6290 + 0x10;
    q = D_L00_001E6280 + 0x10;
    w = D_L00_001E6358 + 1;
    for (i = 8; i >= 0; i--) {
        func_001F9BD8(p, q, h);
        q += 0x10;
        p += 0x10;
        *w = 0;
        w++;
    }
    func_001F9BC0(D_L00_00161C30);
    s[5] = 0;
    s[6] = 0;
    *(float *)((char *)s + 0x6C) = 1.0f;
    *(float *)((char *)s + 0x58) = 0.1f;
    *(float *)((char *)s + 0x5C) = 1.0f;
    *(float *)((char *)s + 0x60) = 1.0f;
    {
        float f = func_00214158();
        func_002156E0(D_L00_00161C40, (char *)s + 0x50, f, (char *)s + 0x60);
        D_L00_00161C50 = f;
    }
    *(float *)&D_L00_00161C20 = 0.006f;
    D_L00_00161C24 = 0;
    D_L00_00161C28 = 0;
    if (*(int *)&D_L00_00161C14 != 0) {
        *(int *)&D_L00_00161C14 = 0;
        for (i = 199; i >= 0; i--) *(int *)(D_L00_001E4FD0_x + i * 0x18 + 0x14) = 0;
    }
}
