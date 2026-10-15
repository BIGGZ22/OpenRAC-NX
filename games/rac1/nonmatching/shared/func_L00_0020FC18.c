/* NON_MATCHING func_L00_0020FC18 -- src/overlays/shared/help_0020CDF0.c
 * Best so far: SIZE ours 1776 / retail 1828, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Per-item attach and advance over 7 slots (stride 0x50) and two moby pointers per slot; attach helpers, a 0x118
 *   Differences: our 'j' sits in $22 and 'p' in $30 where retail has j in $30 and p spilled to 0x64(sp); retail al
 *   Would unblock: a way to make p and slot lose their registers (retail's refs/live ordering), or a reason retail
 */
extern int D_L00_0015F6A8 MACRO_ADDR;
extern int D_L00_0015F770 MACRO_ADDR;
extern int D_L00_0015F6B0 MACRO_ADDR;
extern unsigned char D_0015EEB4_m[4] __asm__("D_0015EEB4");
extern char D_0013E633[];
extern Rec D_L00_00179BC0[] __attribute__((section(".data")));
extern char D_L00_0017A6C0[];
extern char D_L00_0017C580[];
extern char D_L00_0017C780[];
extern char D_L00_0017A708[];
extern char D_L00_0015F788[];
extern char D_L00_0017AA60[];
extern char D_L00_0017A780[];
extern char D_L00_0017A750[];
extern char D_L00_0017C980[];
extern char D_L00_0017A768[];
extern char D_L00_0017CA80[];
extern void func_002153E8(void *, void *);
extern void func_L00_002514B8(void *);
extern void func_L00_00251E30(void *);
int func_L00_0020DB68(int arg);
int func_L00_0020DBB0(int arg);
int func_L00_0020DBD8(int arg);
extern void func_001FA480(void *, void *);
void func_00214F78(f32 *arg0);
extern void func_001F9EC0(void *, void *, void *);
extern float func_001F9CE8(void *);
extern float func_L00_001FF860(float, float);
extern void func_0020EEE8(void *);
extern void func_L00_00209940(void *, void *, void *, int, int);
extern void func_L00_0020F7F8(void *, void *);
extern void func_001F9CA0(void *, void *, void *);
extern void *func_0020D348_m(int) __asm__("func_0020D348");
void func_0020DAF8(char *arg0, int arg1, char *arg2);
void func_L00_00208318(void);
extern int func_001F9850(int);
extern float func_001FA888(int);
f32 func_001F9FA8(f32);
extern int func_001FA898_r(float) __asm__("func_001FA898");

/* Per-item attach and advance: walks the item slots and their moby pointers, attaches or advances each item and sets up the item kept at 0x118C. */
void func_L00_0020FC18(void) {
    int i;
    int j;
    int slot;
    int jmax;
    int koff;
    int kind;
    int a21, a20, a19;
    int r1, r2, r3, r16, c;
    int g;
    float t, fa, x;
    f32 f20;
    f32 vec[8];
    char ta[0x30];
    char tb[0x10];
    char *pa;
    char *pb;
    char *p;
    char *moby;
    char *rec;
    char *q;
    char *s18;

    for (i = 0; i < 7; i++) {
        if ((D_L00_0015F6A8 == 2 || D_L00_0015F6A8 == 6) && i != 4 && i != 5) continue;
        slot = i * 0x50;
        p = D_0013E633 + 0xE1D + slot;
        pa = ta;
        pb = tb;
        jmax = (i == 1 || i == 3 || i == 4) ? 2 : 1;
        for (j = 0; j < jmax; j++) {
            if (j == 1) moby = *(char **)(p + 0x1094);
            else moby = *(char **)(p + 0x1090);
            if (moby != 0) {
                *(u64 *)(moby + 0x38) = *(u64 *)(*(char **)(D_0013E633 + 0xE1D + 0x2080) + 0x38);
                rec = (char *)D_L00_00179BC0 + *(int *)(p + 0x10B8) * 0x4C;
                kind = *(int *)(rec + 0xC);
                if (i == 1 && j == 1) kind = 3;
                koff = kind << 6;
                if (*(unsigned char *)(p + 0x10AA) != 0) {
                    qcopy(p + 0x1070, D_0013E633 + 0xE1D + 0xAF0 + koff);
                    func_002153E8(D_0013E633 + 0x18DD + koff, p + 0x1080);
                    func_L00_002514B8(moby);
                    func_L00_00251E30(moby);
                    *(u16 *)(moby + 0x34) &= 0xFFFB;
                } else {
                    qcopy(moby + 0x10, D_0013E633 + 0xE1D + 0xAF0 + koff);
                    if (D_L00_0015F770 != 0 && *(int *)(rec + 0x18) != 0) {
                        func_L00_002514B8(moby);
                        if (D_L00_0015F770 == 0 || i != 0) {
                            func_002153E8(D_0013E633 + 0x18DD + koff, moby + 0x40);
                        } else {
                            func_L00_0020F7F8(moby, D_0013E633 + 0x18DD + koff);
                        }
                        func_L00_00251E30(moby);
                        if (D_0015EEB4_m[1] != 0) {
                            func_001F9CA0(moby + 0xE0, moby + 0xC0, moby + 0xD0);
                        }
                        func_0020EEE8(moby);
                        *(u16 *)(moby + 0x34) |= 6;
                    } else {
                        a21 = func_L00_0020DB68(i);
                        a20 = func_L00_0020DBB0(i);
                        a19 = func_L00_0020DBD8(i);
                        if (!a21 && !a20 && !a19) func_L00_002514B8(moby);
                        func_L00_00251E30(moby);
                        q = moby + 0xC0;
                        func_001FA480(q, D_0013E633 + 0x18DD + koff);
                        if (!a21 && !a20 && !a19) func_00214F78((f32 *)q);
                        f20 = 1.0f;
                        vec[0] = f20;
                        vec[1] = 0.0f;
                        vec[2] = 0.0f;
                        func_001F9EC0(vec, vec, q);
                        t = func_001F9CE8(vec);
                        *(f32 *)(moby + 0x44) = -func_L00_001FF860(t, vec[2]);
                        *(f32 *)(moby + 0x48) = func_L00_001FF860(vec[0], vec[1]);
                        func_0020EEE8(moby);
                        *(u16 *)(moby + 0x34) |= 6;
                        if (a21) {
                            func_L00_00209940(D_L00_0017A6C0, D_L00_0017C580, *(void **)(moby + 0x24), 0, 0);
                            *(int *)(moby + 0x54) = 0;
                            *(int *)(moby + 0x50) = 0;
                            *(char **)(moby + 0x68) = D_L00_0017C580;
                            *(char **)(moby + 0x6C) = D_L00_0017C580;
                        } else if (a20) {
                            if (*(short *)(moby + 0xA6) == 0x1B1) {
                                func_L00_00209940(D_L00_0017A708, D_L00_0017C780, *(void **)(moby + 0x24), 6, 0);
                            } else {
                                func_L00_00209940(D_L00_0015F788, D_L00_0017C780, *(void **)(moby + 0x24), 0, 0);
                            }
                            *(int *)(moby + 0x54) = 0;
                            *(char **)(moby + 0x6C) = D_L00_0017C780;
                            *(char **)(moby + 0x68) = D_L00_0017C780;
                            *(int *)(moby + 0x50) = 0;
                        } else if (a19) {
                            qcopy(vec, D_L00_0017AA60);
                            qcopy((char *)vec + 0x10, D_L00_0017AA60 + 0xB0);
                            *(f32 *)(D_L00_0017A780 + 0x398) = f20;
                            *(f32 *)(D_L00_0017A780 + 0x2E0) = f20;
                            *(f32 *)(D_L00_0017A780 + 0x2E4) = f20;
                            *(f32 *)(D_L00_0017A780 + 0x2E8) = f20;
                            *(f32 *)(D_L00_0017A780 + 0x390) = f20;
                            *(f32 *)(D_L00_0017A780 + 0x394) = f20;
                            if (j != 0) {
                                func_L00_00209940(D_L00_0017A768, D_L00_0017CA80, *(void **)(moby + 0x24), 0, 0);
                                *(char **)(moby + 0x68) = D_L00_0017CA80;
                                *(char **)(moby + 0x6C) = D_L00_0017CA80;
                            } else {
                                func_L00_00209940(D_L00_0017A750, D_L00_0017C980, *(void **)(moby + 0x24), 0, 0);
                                *(char **)(moby + 0x68) = D_L00_0017C980;
                                *(char **)(moby + 0x6C) = D_L00_0017C980;
                            }
                            *(int *)(moby + 0x54) = 0;
                            *(int *)(moby + 0x50) = 0;
                            qcopy(D_L00_0017AA60, vec);
                            qcopy(D_L00_0017AA60 + 0xB0, (char *)vec + 0x10);
                        }
                    }
                }
            }
            if (i == 3 && j == 1 && moby != 0) {
                char *base = D_0013E633 + 0xE1D;
                if (*(char **)(base + 0x118C) == 0) {
                    s18 = func_0020D348_m(0x4B4);
                    if (s18 == 0) continue;
                    *(char **)(base + 0x118C) = s18;
                    *(u32 *)(s18 + 0x90) = 0x801432D7;
                    qcopy(s18 + 0x10, base + 0x80);
                    qcopy(s18 + 0x40, base + 0x90);
                    *(u16 *)(s18 + 0x32) = 0x40;
                    *(unsigned char *)(s18 + 0x31) = 1;
                    *(unsigned char *)(s18 + 0x30) = 0;
                    *(u16 *)(s18 + 0x34) = 0;
                    *(f32 *)(s18 + 0x2C) = *(f32 *)(s18 + 0x2C) * 1.3f;
                    func_L00_00251E30(s18);
                    if (*(char **)(base + 0x118C) == 0) continue;
                }
                s18 = *(char **)(base + 0x118C);
                if (((*(u16 *)(s18 + 0x34)) ^ 1) & 1) {
                    func_0020DAF8(moby, 6, pa);
                    qcopy(base + 0x1D80, pb);
                    func_L00_00208318();
                    qcopy(s18 + 0x10, pb);
                    func_001FA480(s18 + 0xC0, pa);
                    func_L00_00251E30(s18);
                    *(u64 *)(s18 + 0x38) = *(u64 *)(*(char **)(base + 0x2080) + 0x38);
                    r16 = func_001F9850(0x78);
                    fa = func_001FA888(r16);
                    g = D_L00_0015F6B0;
                    x = (float)(g % r16) / fa;
                    x = x + x;
                    x = x * 3.14159265f;
                    x = x + -3.14159265f;
                    f20 = func_001F9FA8(x);
                    r1 = func_001FA898_r(f20 * 90.0f);
                    r2 = func_001FA898_r(f20 * 50.0f);
                    r3 = func_001FA898_r(f20 * 10.0f);
                    c = r1 + 215;
                    if (!(c < 256)) c = 0xFF;
                    *(u32 *)(s18 + 0x90) = 0x80000000 | ((r3 + 20) << 16) | ((r2 + 50) << 8) | c;
                    *(u16 *)(s18 + 0x34) |= 0x10;
                }
            }
        }
    }
}
