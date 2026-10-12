/* from src/core/0011E938.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/00119D88.c: func_0011BEB8 */
/* Three-way compare-and-confirm: the slot r1_D_001580A8 has to agree with
   the scratch buffer r1_D_0012FCEC, then with the handle r1_D_0012FDA8, and
   finally the two of those with each other. Only the last comparison's
   result is reported, as a 0/1.

   The two globals are bound to locals, buffer first, purely to fix the
   order the compiler materialises their %hi halves in: used directly,
   the slot's lui comes first and the function is 4/140. Declaration
   order of the locals decides it. */
int func_0011EA38(void) {
    char *buf = r1_D_0012FCEC;
    char *slot = r1_D_001580A8;
    int r = 0;

    if (r1_func_001151B4(slot, buf, 4) != 0 &&
        r1_func_001151B4(slot, r1_D_0012FDA8, 4) != 0) {
        r = r1_func_001151B4(buf, r1_D_0012FDA8, 4) != 0;
    }
    return r;
}
