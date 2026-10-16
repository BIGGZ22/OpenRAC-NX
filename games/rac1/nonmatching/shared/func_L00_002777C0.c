/* NON_MATCHING func_L00_002777C0 -- src/overlays/shared/pause_00277208.c
 * Best so far: SIZE ours 700 / retail 708, checked 2026-10-05.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * No longer builds in its file (COMPILE failed, 2026-10-09): match its declarations to the file's first.
 * What the last attempts found:
 *   Page-menu close (PageMenuClose): if page state (+0x110) >= 10, runs each object's close handler (+0xC) for 14 
 *   Best: p2.c, SIZE 700/708. Body and call order are right (same frame, same 9 saved registers); `x ? x : 0x26` r
 *   Remaining: (1) retail keeps the object-loop base as a separate copy (`daddu $17,$16,$0`, counter in $16); ours
 *   Tried: variables vs folded offsets, declaration order, p = g vs p = symbol. Would unblock: a wording that keep
 *   q28/t01: best.c fails COMPILE because the file declares D_0014171B/D_0013E633 as single objects (`extern unsig
 */
extern void func_00234AC8(int);
extern void func_002348E8(void);
extern void func_001FF958(int, int, int);
extern void func_L00_00235CA0(int);
extern int func_002267C0(int);
extern int func_L00_00235790(void);
extern int func_001F9850(int);
extern void func_L00_002367A8(int, int);
extern int func_001FFB38(int, int, int, int, int, int, int);
extern void func_002349B8(void);
extern void func_L00_0023A658();
extern void func_L00_0023A690();
extern void func_L00_0023A788();
extern char D_L00_001BA070[] NOT_SDA;
extern char D_L00_0017E5D8[];
extern char *D_L00_00197680[] NOT_SDA;
extern int D_L00_00173F40[];
extern unsigned char D_0014171B NOT_SDA;
extern unsigned char D_0013E633 NOT_SDA;
extern int D_0015EE98 MACRO_ADDR;
extern short D_0015EF78_s __asm__("D_0015EF78");
extern int D_L00_0015F6BC MACRO_ADDR;

/* Closes the page menu: runs each object's close handler, restores the camera and input state. */
void func_L00_002777C0(void) {
    char *g = D_L00_001BA070;
    char *p, *q, *s, *m, *t, *t2;
    int *u, *dst, *src, *cur;
    int i, v, r, x, o;
    if (*(int *)(g + 0x110) < 10) return;
    func_00234AC8(1);
    func_002348E8();
    if (*(int *)(g + 4) != 0) {
        char *pp = D_L00_001BA070 + 0;
        int k;
        for (k = 0; k < 14; k++) {
            char *obj = *(char **)(*(char **)(pp + 4) + 0x44 + k * 4);
            if (obj != 0) {
                void (*f)(char *, int) = *(void (**)(char *, int))(obj + 0xC);
                if (f != 0) f(obj, 0);
            }
        }
        q = D_L00_001BA070;
        *(int *)(q + 4) = 0;
    }
    s = D_L00_001BA070;
    dst = (int *)(&D_0013E633 + 0xE1D);
    dst = (int *)((char *)dst + 0x20B8);
    src = (int *)(s + 0x30);
    cur = (int *)(&D_0014171B + 0x45);
    for (i = 0; i < 4; i++) {
        x = src[i];
        if (cur[i] != x) {
            dst[i] = x ? x : 0x26;
        }
    }
    m = D_L00_0017E5D8;
    s = D_L00_001BA070;
    *(int *)&D_0015EF78_s = *(int *)(s + 0x18);
    func_001FF958(0, *(int *)(*(char **)(m + 0x18) + 0x74), 0);
    func_L00_00235CA0(2);
    if (*(int *)(*(char **)(m + 0x18) + 0x80) != 0) func_L00_00235CA0(3);
    if (*(int *)(*(char **)(m + 0x18) + 0x84) != 0) func_L00_00235CA0(4);
    for (i = 0; i < 14; i++) D_L00_001BA220[i] = func_002267C0(D_L00_001BA220[i]);
    o = func_L00_00235790();
    func_L00_002367A8(o, func_001F9850(0xB4));
    o = func_001FFB38(2, 0x754E, (int)func_L00_0023A658, (int)func_L00_0023A690, (int)func_L00_0023A788, (int)&D_0015EE98, 9999999);
    func_L00_002367A8(o, func_001F9850(0xB4));
    *(int *)D_L00_001BA070 = 0x14;
    t = D_L00_001BA070;
    *(int *)(t + 0x14) = 2;
    v = *(int *)(t + 0xCC);
    for (; v < *(unsigned char *)(D_L00_00197680[0] + 0xC); v++) *(int *)(D_L00_00197680[0] + 0x48 + v * 4) = 0;
    t2 = D_L00_001BA070;
    D_L00_00197680[0][0xC] = *(unsigned char *)(t2 + 0xCC);
    D_L00_0015F6BC = 1;
    u = D_L00_00173F40;
    u[4] &= 0x7FFFFFFF;
    func_002349B8();
    func_00234AC8(1);
}
