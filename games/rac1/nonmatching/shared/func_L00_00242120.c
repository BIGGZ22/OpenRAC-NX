/* NON_MATCHING func_L00_00242120 -- src/overlays/shared/loaders_00240398.c
 * Best so far: SIZE ours 428 / retail 432, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Builds a (key,value) table on the stack (512 pairs) from 0x20-byte records, bubble-sorts it by value (descendi
 *   Best: p0.c (428 vs 432 bytes, struct array with i++ in the for header). Loops 2 and 3 already match; the diffe
 *   recomputes sp+$s0 for the key store, uses a hoisted sp+4 ($s4) for arr[i].val and a fresh `addiu $6,$sp,4` for
 *   ours CSEs the sentinel at arr[i]+8 (extra $s5, frame 0x1080) or strength-reduces to a pointer IV (i++ inside t
 */
extern float func_001FA888(int);
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern int func_00120778(float);
extern int func_001E9730();
extern char D_L00_001E89C0[];

struct SortEnt {
    float key;
    float val;
};

/* build a table of (key, value) pairs from 0x20-byte records, sort it by value, and emit each */
void func_L00_00242120(int *p, int n) {
    struct SortEnt arr[512];
    struct SortEnt *q;
    int i, j, k, sorted;

    for (i = 0; i < n; i++, p += 8) {
        arr[i].key = (float)p[1];
        if (p[0] == 0) {
            *(int *)&arr[i].val = 0;
        } else {
            int a = p[8];
            if (a == 0) a = p[0x10];
            arr[i].val = func_001FA888(a - p[0]) * 0.0009765625f;
            arr[i + 1].key = -1.0f;
            *(int *)&arr[i + 1].val = 0;
        }
    }
    do {
        sorted = 1;
        for (j = 0; j < n; j++) {
            if (arr[j].val < arr[j + 1].val) {
                float t0 = arr[j].key;
                float t1 = arr[j].val;
                sorted = 0;
                arr[j].key = arr[j + 1].key;
                arr[j].val = arr[j + 1].val;
                arr[j + 1].key = t0;
                arr[j + 1].val = t1;
            }
        }
    } while (!sorted);
    q = arr;
    for (k = n; k > 0; k--, q++) {
        int a = func_001FA898_r(q->key);
        func_001E9730(D_L00_001E89C0, a, func_00120778(q->val));
    }
}
