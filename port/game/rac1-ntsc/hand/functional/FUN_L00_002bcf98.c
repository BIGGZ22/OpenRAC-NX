/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002BE2A0.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* NON_MATCHING FUN_L00_002bcf98 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 264 / retail 260, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Rebuilds camera tables: if D_L00_001617BC is set, clears it, computes f = D_L00_001616DC / func_001FA6C0(12), 
 *   Wall: D_L00_001617BC is read via lui/lw and written via gp-rel; one .extern size decides both (2 = gp read, 4 
 *   Would need the original's declaration of that flag (likely a header with another type) and the true statement 
 *   mini13/a03 three-identical stop: p7 corrected flag macro/exact DC260 alias, p8 reversed pointer increments, p9
 *   Rebuilds camera tables; remaining source base+16 is folded, initialization lacks retail nop, and trailing poin
 *   ## hq3 s05 round (p10-p15)
 *   Best stays p12 (252/260). Tried: pointer-form second loop with d/e pointers (p10, p11, p12), the store to 1618
 */
extern int D_L00_0016188C_m __asm__("D_L00_001617BC") MACRO_ADDR;
extern short D_L00_001616DC;
extern short D_L00_001617CC;
extern short D_L00_00161718;
extern short D_L00_00161720;
extern short D_L00_00161724;
extern int D_L00_001617D0 MACRO_ADDR;
extern int D_L00_00161818[];
extern int D_L00_00161824 MACRO_ADDR;
extern char D_L00_001DC260_b[] __asm__("D_L00_001DBF60");
extern char D_L00_001DBF50[];
extern char D_L00_001DD0D0[];
extern float func_001FA6C0(int);
extern void func_001F9A68(void *, void *, float);
extern void func_001F9A10(void *, void *, void *);

/* rebuilds the camera tables from a moby's matrix */
void FUN_L00_002bcf98(char *m) {
    char *p = *(char **)(m + 0x78);
    if (D_L00_0016188C_m != 0) {
        float f;
        char v[16];
        int i, j;
        char *a, *b;
        int *e;
        char *d;
        D_L00_0016188C_m = 0;
        f = *(float *)&D_L00_001616DC / func_001FA6C0(12);
        func_001F9A68(v, m + 0xD0, -f);
        *(float *)&D_L00_001617CC = f;
        qcopy(D_L00_001DC260_b, p);
        a = D_L00_001DC260_b + 16;
        b = D_L00_001DBF50 + 16;
        for (i = 10; i >= 0; i--) {
            func_001F9A10(a, b, v);
            b += 16;
            a += 16;
        }
        *(int *)&D_L00_00161718 = 0;
        *(int *)&D_L00_00161720 = 0;
        d = D_L00_001DD0D0;
        *(int *)&D_L00_00161724 = 0;
        D_L00_001617D0 = 0;
        e = D_L00_00161818;
        D_L00_00161824 = 0;
        for (j = 2; j >= 0; j--) {
            *e = 0;
            qcopy(d, v);
            d += 16;
            e++;
        }
    }
}
