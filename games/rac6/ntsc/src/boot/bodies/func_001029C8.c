/* func_001029C8 -- SCUS_976.23 0x001029C8, 8 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   Boot SCUS_976.23 0x001029C8 (8 bytes).
 *   Clear the first byte of the object given as first argument.  The retail
 *   code is `jr $ra` with `sb $zero, 0x0($a0)` in its delay slot.
 *   The layout of the object is only known from the code itself: the byte at
 *   offset 0 is written, nothing else is touched.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Boot_001029C8_Object
{
    unsigned char field_00;
} Boot_001029C8_Object;

void func_001029C8(Boot_001029C8_Object *self)
{
    self->field_00 = 0;
}
