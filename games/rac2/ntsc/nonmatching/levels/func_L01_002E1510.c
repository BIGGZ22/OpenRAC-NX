/* func_L01_002E1510: the retail body (348 bytes) rebuilt from its bytes.
 *
 * The first loop and the call sequence are exact; what is left is allocation and the
 * shape of the outer loop:
 *   - retail copies the first parameter into $s4 and the second into $s6; this cc1 gives
 *     $s5 to the second parameter and $s6 to the first (its first use is the earlier
 *     one, in the guard);
 *   - retail keeps the global base in $t0 and then in $s0, retail's two copies of it;
 *     this cc1 keeps one copy in $s0;
 *   - retail computes the second record pointer as base + offset + 16 and this cc1 as
 *     (base + offset) + 16;
 *   - retail's outer loop head holds the two extra instructions this cc1 folds into the
 *     loop body, so the candidate is 8 bytes short.
 */
extern void func_00283098(void *a0, void *a1, void *a2);
extern void func_00283738(void *a0, void *a1, void *a2);
extern void func_00280B90(void *a0, void *a1, int a2);

void func_L01_002E1510(char *a0, int a1, char *a2, int a3, char *a4) {
    float buf[12];
    char *p;
    char *q;
    char *r;
    int n;
    int i;

    p = a0 + 0x20;
    q = a0 + 0x30;
    if (a1 > 0) {
        n = a1;
        for (; n != 0; n--) {
            func_00283098(p, p, a2);
            p += 0x90;
            func_00283098(q, q, a2);
            q += 0x90;
        }
    }
    func_00283738(buf, a2, a2);
    for (i = 0; i < a3; i++) {
        int k = (i & 1) ? 2 : 0;
        if (a1 > 0) {
            p = a0 + k * 16;
            q = a0 + k * 16 + 16;
            r = a0;
            n = a1;
            while (n != 0) {
                func_00283098(p, p, buf);
                n--;
                p += 0x90;
                func_00283098(q, q, buf);
                q += 0x90;
                func_00280B90(r, a4, 1);
                r += 0x90;
            }
        }
    }
}
