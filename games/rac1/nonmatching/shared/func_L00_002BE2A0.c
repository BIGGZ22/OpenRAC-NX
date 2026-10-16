/* NON_MATCHING func_L00_002BE2A0 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 264 / retail 260, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Rebuilds camera tables: if D_L00_0016188C is set, clears it, computes f = D_L00_001617AC / func_001FA888(12), 
 *   Wall: D_L00_0016188C is read via lui/lw and written via gp-rel; one .extern size decides both (2 = gp read, 4 
 *   Would need the original's declaration of that flag (likely a header with another type) and the true statement 
 *   mini13/a03 three-identical stop: p7 corrected flag macro/exact DC260 alias, p8 reversed pointer increments, p9
 *   Rebuilds camera tables; remaining source base+16 is folded, initialization lacks retail nop, and trailing poin
 *   ## hq3 s05 round (p10-p15)
 *   Best stays p12 (252/260). Tried: pointer-form second loop with d/e pointers (p10, p11, p12), the store to 1618
 */
extern int D_L00_0016188C_m __asm__("D_L00_0016188C") MACRO_ADDR;
extern short D_L00_001617AC;
extern short D_L00_0016189C;
extern short D_L00_001617E8;
extern short D_L00_001617F0;
extern short D_L00_001617F4;
extern int D_L00_001618A0 MACRO_ADDR;
extern int D_L00_001618E8[];
extern int D_L00_001618F4 MACRO_ADDR;
extern char D_L00_001DC260_b[] __asm__("D_L00_001DC260");
extern char D_L00_001DC250[];
extern char D_L00_001DD3D0[];
extern float func_001FA888(int);
extern void func_001F9C30(void *, void *, float);
extern void func_001F9BD8(void *, void *, void *);

/* rebuilds the camera tables from a moby's matrix */
void func_L00_002BE2A0(char *m) {
    char *p = *(char **)(m + 0x78);
    if (D_L00_0016188C_m != 0) {
        float f;
        char v[16];
        int i, j;
        char *a, *b;
        int *e;
        char *d;
        D_L00_0016188C_m = 0;
        f = *(float *)&D_L00_001617AC / func_001FA888(12);
        func_001F9C30(v, m + 0xD0, -f);
        *(float *)&D_L00_0016189C = f;
        qcopy(D_L00_001DC260_b, p);
        a = D_L00_001DC260_b + 16;
        b = D_L00_001DC250 + 16;
        for (i = 10; i >= 0; i--) {
            func_001F9BD8(a, b, v);
            b += 16;
            a += 16;
        }
        *(int *)&D_L00_001617E8 = 0;
        *(int *)&D_L00_001617F0 = 0;
        d = D_L00_001DD3D0;
        *(int *)&D_L00_001617F4 = 0;
        D_L00_001618A0 = 0;
        e = D_L00_001618E8;
        D_L00_001618F4 = 0;
        for (j = 2; j >= 0; j--) {
            *e = 0;
            qcopy(d, v);
            d += 16;
            e++;
        }
    }
}
