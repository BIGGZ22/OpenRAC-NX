/* from src/core/00131780.c: table base reached gp-relative in retail.
 *
 * The slot table at gp - 0x7B50 (0x001A74A0) indexed by the counter at
 * gp - 0x7B30 (0x001A74C0). Retail's base is `addiu $v0,$gp,-0x7B50`; the
 * missing lever was the declaration's SIZE - `int *tbl[]` (incomplete) never
 * takes the gp-relative form, it comes out `lui/addiu` absolute, 8 bytes
 * more. With `int *tbl[2] __attribute__((sda));` (measured 2026-10-15) the
 * gp form is there and the residue is allocation only: our counter lands in
 * `$v0` and the base in `$v1` (retail the other way round), and the call
 * argument costs one extra `daddu $a0,$a1,$zero` - 60 bytes against retail's
 * 56. One more shape from there (an explicit `int **slot` intermediate):
 * same 60. The same size rule is what func_00132DF0's gp accesses needed.
 */
extern int *D_001A74A0[2] __attribute__((sda));

extern int D_001A74C0 __attribute__((sda));

extern void func_00132028_arg(int *arg0) __asm__("func_00132028");

void func_001330D0(void) {
    int *p = D_001A74A0[D_001A74C0];

    (*p)++;
    func_00132028_arg(p);
}
