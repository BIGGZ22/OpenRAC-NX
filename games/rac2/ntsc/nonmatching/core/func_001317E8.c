/* from src/core/00131780.c: C that compiles but does not link at its places yet */
/* rac1-decomp src/core/0012CC90.c: func_0012D500 */
/* Convert the sceCdCLOCK fields (second..year) in place with the helper
   above. Exact under 2.9-ee, which saves $16 before $31 as retail does;
   2.95.3 saved them the other way round (4/104), the prologue-order
   residual that pointed at the SDK compiler in the first place. */
void func_001317E8(unsigned char *p) {
    p[7] = func_00131760(p[7]);
    p[6] = func_00131760(p[6]);
    p[5] = func_00131760(p[5]);
    p[3] = func_00131760(p[3]);
    p[2] = func_00131760(p[2]);
    p[1] = func_00131760(p[1]);
}
