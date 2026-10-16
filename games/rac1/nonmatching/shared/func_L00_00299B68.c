/* NON_MATCHING func_L00_00299B68 -- src/overlays/shared/tieproc_00299108.c
 * Best so far: SIZE ours 780 / retail 772, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Vendor screen entry: resets buffers/state, flags mobys, then waits for the vendor state to reach 3 and runs a 
 *   Needs: D_L00_0015F6BC/0015F500/0016128C stored gp-relative (declare `extern short ... __asm__("...")` alias) b
 *   Remaining difference: retail keeps `h = D_0014171B+0x100B5` in $a2 with a copy in $s1 for the wait loop (and l
 */
#include "common.h"
#include "include_asm.h"

extern int D_L00_0015F6A8_m __asm__("D_L00_0015F6A8") MACRO_ADDR;
extern short D_L00_0015F6BC_s __asm__("D_L00_0015F6BC");
extern int D_L00_0015F4FC_m __asm__("D_L00_0015F4FC") MACRO_ADDR;
extern short D_L00_0015F500_s __asm__("D_L00_0015F500");
extern int D_L00_0016128C_i __asm__("D_L00_0016128C");
extern short D_L00_0016128C_s __asm__("D_L00_0016128C");
extern int D_L00_0015F67C_m __asm__("D_L00_0015F67C") MACRO_ADDR;
extern int D_L00_0015F680_m __asm__("D_L00_0015F680") MACRO_ADDR;
extern char D_0013E633[];
extern unsigned char D_0014171B[] NOT_SDA;
extern char D_L00_0016C960[];
extern char D_L00_0017C540[];
extern char D_L00_0017C440[];
extern char D_L00_0017C480[];
extern char D_L00_0017C500[];
extern char D_L00_0017C4C0[];
extern int D_L00_00173F00[];
extern unsigned char D_L00_00166D80_u[] __asm__("D_L00_00166D80");
extern char *D_L00_00179208;
extern char *D_L00_001BA15C;
extern unsigned char *D_L00_0016009C_m __asm__("D_L00_0016009C") MACRO_ADDR;
extern void func_L00_00211908(void);
extern void func_00216EF0(int);
extern void func_001F99B0(void *, int, int);
extern void func_L00_00203FB8(void);
extern void func_L00_00233868(void);
extern void func_L00_00245B88(int);
extern int func_001F9850(int);
extern void func_001F4E08(int);
extern void func_00204FC0_v(void) __asm__("func_00204FC0");
extern void func_0022DD68(void);
extern void func_00122598(int);

// Sets up the vendor screen state from the requested entry and runs it until the screen closes.
void func_L00_00299B68(int arg) {
    char *d;
    unsigned char *p;
    char *h;
    char *q;
    int i;
    int v;
    float one;
    if (arg < 0) return;
    if (D_L00_0015F6A8_m == 2) return;
    d = (char *)D_0013E633 + 0xE1D;
    if (*(int *)(d + 0x22A8) == 0) {
        func_L00_00211908();
        return;
    }
    func_00216EF0(0);
    func_00217748(0);
    *(int *)&D_L00_0015F6BC_s = 1;
    func_001F99B0(D_L00_0016C960, 0, 0x1C0);
    func_001F99B0(D_L00_0017C540, 0, 0x40);
    func_001F99B0(D_L00_0017C440, 0, 0x40);
    func_001F99B0(D_L00_0017C480, 0, 0x40);
    func_001F99B0(D_L00_0017C500, 0, 0x40);
    func_001F99B0(D_L00_0017C4C0, 0, 0x40);
    *(int *)(D_L00_0016C960 + 0x30) = arg;
    *(int *)(D_L00_0016C960 + 0x58) = D_L00_00173F00[1] + (D_L00_0016128C_i - 0x3C000);
    *(int *)(D_L00_0016C960 + 0x5C) = D_L00_00173F00[2] + (D_L00_0016128C_i - 0x3C000);
    one = 1.0f;
    *(float *)&D_L00_0015F67C_m = one;
    D_L00_0015F680_m = 0;
    *(int *)(D_L00_0016C960 + 0x34) = 0;
    *(int *)(D_L00_0016C960 + 0x3C) = 0;
    *(int *)&D_L00_0016128C_s = D_L00_0016128C_i - 0x3C000;
    func_L00_00203FB8();
    *(float *)&D_L00_0015F4FC_m = one;
    D_L00_0015F6A8_m = 2;
    D_L00_0016C960[0x4B] = D_L00_00166D80_u[0x394];
    *(int *)(D_L00_00166D80_u + 0x394) = 0;
    *(int *)&D_L00_0015F500_s = 0;
    func_L00_00222B80(0x64, 2);
    d[0x20A5] = 1;
    func_L00_00233868();
    if (D_L00_00179208 != 0) *(unsigned short *)(D_L00_00179208 + 0x34) |= 1;
    if (D_L00_001BA15C != 0) *(unsigned short *)(D_L00_001BA15C + 0x34) |= 1;
    func_L00_00245B88(0);
    func_001F4E08(func_001F9850(6));
    func_00217748(1);
    func_00204FC0_v();
    for (p = D_L00_0016009C_m; p[0x20] != 0xFF; p += 0x100) {
        if (p[0x20] & 0x80) continue;
        if (*(short *)(p + 0xA6) == 0x4A || *(short *)(p + 0xA6) == 0xCB) {
            *(unsigned short *)(p + 0x34) |= 0x80;
        }
    }
    h = (char *)D_0014171B + 0x100B5;
    *(int *)(h + 0x1C) = *(int *)(D_L00_0016C960 + 0x30);
    while (*(short *)(h + 0x5A) != 3) {
        func_0022DD68();
        func_00122598(0);
    }
    q = D_L00_0016C960;
    *(short *)(q + 0x46) = -3;
    func_00216960();
    func_L00_00245B88(1);
    for (i = 0; *(short *)(q + 0x46) < i; i--) {
        func_00122598(0);
    }
}
