/* func_00103608 -- SCUS_976.23 0x00103608, 8 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   Boot SCUS_976.23 0x00103608 (8 bytes).
 *   Setter: stores the second argument in the word at offset 0x60 of the
 *   object given as first argument.  The retail code is `jr $ra` with
 *   `sw $a1, 0x60($a0)` in its delay slot.
 *   The layout of the object is only known from the code itself: the word at
 *   offset 0x60 is written, nothing else is touched.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Boot_00103608_Object
{
    unsigned char pad_000[0x60];
    int          field_060;
} Boot_00103608_Object;

void func_00103608(Boot_00103608_Object *self, int value)
{
    self->field_060 = value;
}
