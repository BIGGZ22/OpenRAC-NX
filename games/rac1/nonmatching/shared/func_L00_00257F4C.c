/* NON_MATCHING func_L00_00257F4C -- src/overlays/shared/mobyproc_00251A78.c
 * Best so far: SIZE ours 112 / retail 272, checked 2026-10-15.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   `extern char *D_L00_0015FC80 MACRO_ADDR;`, which matches the usage.
 *   Remaining difference: see the BYTES 86/104 result above (`lq`/`prot3w`
 *   vector shape, and a `jal` in place of retail's out-of-function `bnez`).
 *   2026-10-15: since cfa93640 this is joined with func_L00_00257FB4 and func_L00_0025804C (272 bytes), and
 *   those pieces are handwritten assembly: prot3w, VU0 macro instructions (lqc2, vsub, vmul, qmfc2.ni), addi
 *   marked handwritten by the disassembler, hand-placed nops. No C compiles to that, so this should stay
 *   assembly. Levels have no handwritten list yet (docs/ASM_CLASSIFICATION.md lists only remnants), which is
 *   an upstream decision.
 */
/* Grid lookup: the 64 x 64 cell of the point (v[0], v[1]) (16 units a cell)
   holds an offset into the table at D_L00_0015FC80; a nonzero offset hands
   its list on to func_L00_00257FB4, zero returns 0. */
typedef struct { float f[4]; } Vec4 __attribute__((aligned(16)));

extern char *D_L00_0015FC80 MACRO_ADDR;
extern int func_L00_00257FB4(void *, void *);

int func_L00_00257F4C(Vec4 *p) {
    Vec4 v;
    unsigned int xi;
    unsigned int yi;
    int value;

    v = *p;
    xi = (unsigned int)(int)v.f[0] >> 4;
    yi = (unsigned int)(int)v.f[1] >> 4;
    value = ((int *)D_L00_0015FC80)[(yi << 6) + xi];
    if (value != 0) {
        return func_L00_00257FB4(p, D_L00_0015FC80 + value);
    }
    return 0;
}
