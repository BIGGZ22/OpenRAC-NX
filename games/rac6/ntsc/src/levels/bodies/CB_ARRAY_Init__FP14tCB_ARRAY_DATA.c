/* CB_ARRAY_Init__FP14tCB_ARRAY_DATA -- FE.REL 0x068548, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   CB_ARRAY_Init__FP14tCB_ARRAY_DATA  (FE.REL 0x068548, 32 bytes)
 *   Retail:  addiu $sp,-0x10 / daddu $a1,$zero,$zero / sd $ra
 *   jal memset / addiu $a2,$zero,0x84 / ld $ra / jr $ra / addiu $sp,0x10
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tCB_ARRAY_DATA tCB_ARRAY_DATA;

extern void *memset(void *dst, int value, unsigned int count);

void CB_ARRAY_Init__FP14tCB_ARRAY_DATA(tCB_ARRAY_DATA *data)
{
    memset(data, 0, 0x84);
}
