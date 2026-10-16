/* NON_MATCHING func_L00_00257F4C -- src/overlays/shared/mobyproc_00251A78.c
 * Best so far: SIZE ours 112 / retail 104, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   shape or the call form. Neither can make the outside branch, so no EXACT is
 *   reachable with this boundary. Lombyte has no checkout here (`tools/lombyte.py`
 *   says so), so there was nothing to port.
 *   Naming: `strings.tsv` labels 0x15FC80 as 'hud bank 0' (func_002032D0). Here
 *   it is a pointer to an int table, not a string. Loaders_00240398.c has
 *   `extern char *D_L00_0015FC80 MACRO_ADDR;`, which matches the usage.
 *   Remaining difference: see the BYTES 86/104 result above (`lq`/`prot3w`
 *   vector shape, and a `jal` in place of retail's out-of-function `bnez`).
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
