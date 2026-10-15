/* NON_MATCHING func_L00_0024A798 -- src/overlays/shared/menu_00249720.c
 * Best so far: BYTES 7/288 (97.6% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   LRU cache lookup of 8 entries (key/stamp/dst, 16 bytes each); swaps 128-bit entry i with entry 0 in D_L00_0018
 *   p4.c: all matches except the 128-bit swap register allocation (v0/a0/v1 vs a0/v0/v1, 6 instructions). Budget s
 */
typedef int u128 __attribute__((mode(TI)));
typedef struct {
    int key;
    int stamp;
    unsigned char *dst;
    int pad;
} CacheEnt;
typedef struct {
    unsigned char *src;
    short *offs;
    int pad0[8];
    int active;
    int pad1;
    CacheEnt e[8];
} Cache;
extern void func_00207090(unsigned char *dst, int row, unsigned char *src, short *offs);
extern Cache D_L00_001842F0;
extern u128 D_L00_00184320[];
extern int D_L00_0015F4F8 MACRO_ADDR;

/* Finds or loads a cached slot for key (LRU replacement) and moves it to the front. */
int func_L00_0024A798(int key) {
    int ret;
    int slot;
    int i;
    if (D_L00_001842F0.active == 0) {
        return 0;
    }
    ret = 0;
    slot = 0;
    for (i = 0; i < 8 && D_L00_001842F0.e[i].key != key; i++) {
        if (D_L00_001842F0.e[i].stamp < D_L00_001842F0.e[slot].stamp) {
            slot = i;
        }
    }
    if (i == 8) {
        func_00207090(D_L00_001842F0.e[slot].dst, key, D_L00_001842F0.src, D_L00_001842F0.offs);
        D_L00_001842F0.e[slot].key = key;
        i = slot;
        ret = 1;
    }
    if (i != 0) {
        u128 a = D_L00_00184320[0];
        u128 b = D_L00_00184320[i];
        D_L00_00184320[i] = a;
        D_L00_00184320[0] = b;
    }
    D_L00_001842F0.e[0].stamp = D_L00_0015F4F8;
    return ret;
}
