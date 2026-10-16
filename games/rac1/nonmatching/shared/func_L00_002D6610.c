/* NON_MATCHING func_L00_002D6610 -- src/overlays/shared/vendor_002D1168.c
 * Best so far: SIZE ours 876 / retail 868, checked 2026-10-08.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Per-frame vendor-path update (state machine on m[0x20], calls into 0025B478/0025B4D0/0025BBA0/0025D5B0/00260FB
 *   Differences: register permutation (retail $16=d+0x70 pointer, $17=d, $18=m; ours $16=d, $17=m, $18=pointer); t
 *   Would unblock: a way to raise the d+0x70 pointer's allocation priority without a second variable of the same v
 */
extern char *func_L00_0025B478(void *, int, int);
extern int func_L00_0025B4D0(void *, void *, void *, int, int *, float *, int, int);
extern void func_L00_0025BBA0(void *, float *, void *, void *);
extern int func_002140B0(int);
extern void func_L00_0025D5B0(void *, void *, float, int, int, int);
extern void func_L00_0025E4B0(void *m, short *p);
extern void func_L00_002584A8(void *, int, int);
extern void func_L00_0025E590(void *, void *);
extern int func_L00_00260FB0(float, char *, void *, int, int, void *, int);
extern int func_001F9938(void *);
extern float func_002140F8(float, float);
extern float func_001F9878(float);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_001F9850(int);
extern void func_00213DE0(void *, int, int, int);
extern int D_L00_0015F6A8 MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern short D_L00_00161A40;
extern short D_L00_00161A44;
extern short D_L00_00161A48;
extern short D_L00_00161A4C;
extern short D_L00_00161A58;
extern short D_L00_00161A5C;
extern char *D_L00_001B0830[];
extern char D_0013E633[];
typedef int u128 __attribute__((mode(TI)));

/* Per-frame update of a vendor-path moby: picks its path entry, updates its state and calls the helpers. */
void func_L00_002D6610(unsigned char *m) {
    unsigned char *d;
    unsigned char *q;
    unsigned char *c;
    unsigned char *p;
    unsigned char st;
    unsigned short h;
    char *e;
    u128 vec;
    float f;
    float g;
    int r;
    int idx;
    int t;
    int k;

    if (m[0x20] == 0) return;
    d = *(unsigned char **)(m + 0x78);
    h = *(unsigned short *)(m + 0x34);
    if (D_L00_0015F6A8 == 2) {
        m[0x31] = 0;
        h |= 1;
    } else {
        m[0x31] = 1;
        h &= 0xFFFE;
    }
    *(unsigned short *)(m + 0x34) = h;
    f = 0.0f;
    p = (unsigned char *)func_L00_0025B478(m, 0x330000, 0);
    c = d + 0x60;
    func_L00_0025B4D0(m, p, d + 0x20, 0, &r, &f, 0, 4);
    k = 0xC;
    if (r != 1 && m[0x20] != k && f != 0.0f) {
        *(float *)(d + 0x80) = *(float *)&D_L00_00161A40 * D_0015EE70;
        *(float *)(d + 0x84) = *(float *)&D_L00_00161A44 * D_0015EE70;
        *(float *)(d + 0x88) = *(float *)&D_L00_00161A48 * D_0015EE6C;
        *(float *)(d + 0x8C) = *(float *)&D_L00_00161A4C * D_0015EE6C;
        *(float *)(d + 0x20) = *(float *)(d + 0x20) - f;
        *(float *)(d + 0x98) = 0.75f;
        *(float *)(d + 0xBC) = D_0015EE6C + D_0015EE6C;
        *(int *)(d + 0x94) = 0x29;
        *(int *)(d + 0x90) = 0x200;
        d[0xAD] = 0;
        *(unsigned short *)(m + 0x34) = *(unsigned short *)(m + 0x34) & 0xEFFF;
        vec = *(u128 *)(p + 0x10);
        func_L00_0025BBA0(&vec, &g, d + 0x88, d + 0x8C);
        q = d + 0x70;
        t = func_002140B0(2);
        func_L00_0025D5B0(m, q, g, t + 6, 1, 0);
        *(float *)(d + 0xC0) = *(float *)&D_L00_00161A5C;
        *(float *)(d + 0xC4) = *(float *)&D_L00_00161A58;
        d[0x67] = 0x78;
        m[0x20] = k;
        func_L00_0025E4B0(m, (short *)c);
        func_L00_002584A8(m, 0, -1);
    }
    m[0xA4] = 0xFF;
    func_L00_0025E590(m, c);
    q = d + 0x180;
    if (m[0x20] == 3) {
        idx = *(int *)(d + 0x1E4);
        e = D_L00_001B0830[idx];
        func_L00_00260FB0(64.0f, (char *)m, q, 0, 0, e + 0x10, *(int *)e);
    } else {
        idx = *(int *)(d + 0x1E0);
        e = D_L00_001B0830[idx];
        func_L00_00260FB0(24.0f, (char *)m, q, 0, 0, e + 0x10, *(int *)e);
    }
    if (*(int *)(d + 0x1C0) == 0) {
        char *s = D_0013E633 + 0xE1D;
        *(int *)(d + 0x1C0) = *(int *)(s + 0x2080);
        *(u128 *)q = *(u128 *)(s + 0x80);
    }
    func_001F9938(d + 0x276);
    if (*(int *)(d + 0x38) != 0) {
        float a = func_002140F8(180.0f, 240.0f);
        float b = func_001F9878(a);
        *(short *)(d + 0x276) = func_001FA898_r(b);
    }
    *(int *)(d + 0x38) = 0;
    st = m[0x20];
    if (st == 5 || st == 8) {
        if (*(short *)(d + 0x276) != 0) {
            m[0x20] = 10;
            if (m[0x53] != 4) {
                t = func_001F9850(0x14);
                func_00213DE0(m, 4, 0, t);
            }
        }
    }
}
