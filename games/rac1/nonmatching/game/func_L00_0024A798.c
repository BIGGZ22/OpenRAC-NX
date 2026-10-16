/* Lookup in an 8-entry table of 16-byte records at D_L00_001842F0 (key at +0x30, value at +0x34,
   extra word at +0x38), keyed by an int. If the table is in use (+0x28 != 0): the key is searched
   among the records; if absent, the record with the smallest value is replaced (func_00207090 is
   called with its extra word first), and the record that was found or replaced is swapped to the
   front of the 16-byte table at D_L00_00184320. The value at +0x34 of the first record gets
   D_L00_0015F4F8. Returns 1 when a record was replaced, 0 otherwise. */
typedef int q128 __attribute__((mode(TI)));
extern char D_L00_001842F0[];
extern char D_L00_00184320[];
extern int D_L00_0015F4F8 MACRO_ADDR;
extern void func_00207090_ovl(int, int, int, int) __asm__("func_00207090");

int func_L00_0024A798(int key) {
    char *A = D_L00_001842F0;
    int i = 0;
    int best = 0;
    int ret = 0;
    int idx;
    char *p5;
    char *p7;

    if (*(int *)(A + 0x28) == 0) {
        return 0;
    }
    p5 = A + 0x30;
    p7 = A + 0x34;
    if (*(int *)p5 != key) {
        do {
            int cand = *(int *)(p5 + 4);
            int cur = *(int *)(p7 + 16 * best);
            if (cand < cur) {
                best = i;
            }
            i++;
            if (i >= 8) {
                break;
            }
            p5 += 16;
        } while (*(int *)p5 != key);
    }
    idx = i;
    if (idx == 8) {
        ret = 1;
        func_00207090_ovl(*(int *)(A + 16 * best + 0x38), key, *(int *)A, *(int *)(A + 4));
        *(int *)(A + 16 * best + 0x30) = key;
        idx = best;
    }
    if (idx != 0) {
        q128 *p = (q128 *)D_L00_00184320;
        q128 *q = (q128 *)(D_L00_00184320 + 16 * idx);
        q128 t = *p;
        *p = *q;
        *q = t;
    }
    *(int *)(A + 0x34) = D_L00_0015F4F8;
    return ret;
}
