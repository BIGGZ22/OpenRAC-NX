/* NON_MATCHING func_L14_00303270 -- src/overlays/shared/vendor_002B2A28.c
 * Best so far: SIZE ours 448 / retail 444, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Chain of vectors advanced per tick (func_001F9BD8 loops), copies 4 indexed quads, then interpolates a value fr
 *   p4.c is 436 vs 444 bytes: remaining diffs are two small moves (retail `daddu $v0,$v1,$zero` in the j loop arou
 */
typedef int u128_03270 __attribute__((mode(TI)));
extern void func_001F9BD8(void *, void *, void *);
extern float func_001FA888(int);
extern int func_001FA8A8(int, int, float);
extern short D_L14_00162194;
extern short D_L14_00162198;
extern short D_L14_001621C4;
extern short D_L14_001621C0;
extern short D_L14_001621C8;

// advances a timed chain of vectors and picks an interpolation value
void func_L14_00303270(char *moby) {
    char *d = *(char **)(moby + 0x78);
    if (*(short *)(d + 0x12) != 0) {
        int i;
        int j;
        int k;
        char *p;
        char *q;
        u128_03270 *dst;
        unsigned char *idx = (unsigned char *)(d + 0x18);
        *(short *)(d + 0x12) = *(short *)(d + 0x12) - 1;
        p = d + 0x260;
        q = d + 0x60;
        for (i = 0x16; i >= 0; i--) {
            func_001F9BD8(q, q, p);
            q += 0x10;
            p += 0x10;
        }
        j = 0;
        do {
            char *base = d + j * 16;
            char *a = base + 0x460;
            char *b = base + 0x560;
            j++;
            for (i = 2; i >= 0; i--) {
                func_001F9BD8(a, a, b);
                a += 0x40;
                b += 0x40;
            }
        } while ((float)j < 4.0f);
        dst = (u128_03270 *)(d + 0x420);
        for (k = 0; k < 4; k++) {
            *dst = ((u128_03270 *)(d + 0x20))[idx[k]];
            dst++;
        }
    }
    {
        int h = *(short *)(d + 0x12);
        if (*(int *)&D_L14_00162194 < h) {
            float f = func_001FA888(h - *(int *)&D_L14_00162194) / func_001FA888(*(int *)(d + 0x620) - *(int *)&D_L14_00162194);
            *(int *)(d + 0x14) = func_001FA8A8(*(int *)&D_L14_001621C4, *(int *)&D_L14_001621C0, f);
        } else if (*(int *)&D_L14_00162198 < h) {
            float f = func_001FA888(h - *(int *)&D_L14_00162198) / func_001FA888(*(int *)&D_L14_00162194 - *(int *)&D_L14_00162198);
            *(int *)(d + 0x14) = func_001FA8A8(*(int *)&D_L14_001621C8, *(int *)&D_L14_001621C4, f);
        } else {
            *(int *)(d + 0x14) = 0;
        }
    }
}
