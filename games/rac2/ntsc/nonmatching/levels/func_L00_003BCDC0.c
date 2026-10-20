/* func_L00_003BCDC0 (100 bytes, 27 places) - best attempt, NOT MATCHED.
 *
 * Docker-free ruler (scratch/wslcmp.py): "00_aranos_tutorial: 108 bytes, expected 100".
 * The walk, the call and the call arguments are right; two measured differences:
 *   (a) mine walks with an end pointer and compares `p < end`, which this cc1 renders
 *       `sltu $v0,$s0,$s2`; retail has the SIGNED `slt $v0,$s0,$s2` - so the original
 *       compared integers, not pointers;
 *   (b) mine materialises 0x23F060+0xB00 first and derives the base with
 *       `addiu $s0,$s2,-0xB00`, retail materialises the base (`lui $v0,0x24 ;
 *       addiu $s0,$v0,-0xFA0`) and then adds 0xB00 for the limit.
 * The extra instruction and the loop rotation follow from (b).
 *
 * To retake: a form whose limit expression is not built from the same base literal, and
 * whose comparison is signed (e.g. an index loop over a signed counter, or the addresses
 * held in `int` locals).
 */

extern void func_L00_003BCE28(char *a0, char *a1);

extern char D_0023F060[];

void func_L00_003BCDC0(char *a0)
{
    char *p;
    char *end = D_0023F060 + 0xB00;

    for (p = D_0023F060; p < end; p += 0xB0) {
        if (*(int *)(p + 0xA0) == (int)a0) {
            func_L00_003BCE28(a0, p);
        }
    }
}
