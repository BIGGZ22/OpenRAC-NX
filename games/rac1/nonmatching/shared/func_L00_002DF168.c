/* NON_MATCHING func_L00_002DF168 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 672 / retail 680, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   2026-10-15: the hero through Hero (state.current, moby->flags, unk22C8, unk640[3]) and the transform loop
 *   as an index loop over D_L00_001E6290/D_L00_001E6280 (i from 1 to 9) give 672 of 680 bytes. The loop body
 *   now matches. Left: retail keeps a in $16, reuses $16 for &s[3] after a dies, then copies it to $20 for the
 *   loop and counts in $16. Ours keeps &s[3] in its own saved register. Passing &s[3] directly instead of h
 *   changes nothing.
 */
#include "hero.h"
extern Hero hero_2DF168 __asm__("D_0013F450");
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
    char *v, *h, *p, *q;
    short *w;
    int i, st;
    if (*(int *)&D_L00_00161C10 == 0) return;
    D_L00_00161C54 = 1.7f;
    *(int *)&D_L00_00161C10 = 0;
    st = hero_2DF168.state.current;
    if ((st == 0x1E || st == 1) && (hero_2DF168.moby->flags & 1)) {
        qcopy((char *)s, D_L00_001670D0);
        qcopy((char *)s + 0x10, D_L00_001670D0 + 0x10);
        v = (char *)s + 0x20;
        qcopy(v, D_L00_001670D0 + 0x20);
    } else {
        if (hero_2DF168.unk22C8 == 1) {
            func_001F9C30(s, a + 0xD0, -1.0f);
            qcopy((char *)s + 0x10, a + 0xC0);
            v = (char *)s + 0x20;
            qcopy(v, a + 0xE0);
        } else {
            qcopy((char *)s, &hero_2DF168.unk640[0]);
            qcopy((char *)s + 0x10, &hero_2DF168.unk640[1]);
            v = (char *)s + 0x20;
            qcopy(v, &hero_2DF168.unk640[2]);
        }
    }
    h = (char *)s + 0x30;
    func_L00_001FF4B0(h, s, D_L00_00161C54);
    func_L00_001FF4B0((char *)s + 0x40, v, 1.0f);
    qcopy(D_L00_001E6290, b);
    D_L00_001E6358[0] = 0;
    for (i = 1; i < 10; i++) {
        func_001F9BD8(D_L00_001E6290 + i * 0x10, D_L00_001E6280 + i * 0x10, h);
        D_L00_001E6358[i] = 0;
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
