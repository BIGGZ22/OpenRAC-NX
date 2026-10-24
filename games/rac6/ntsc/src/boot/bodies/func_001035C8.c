/* func_001035C8 -- SCUS_976.23 0x001035C8, 8 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   Boot SCUS_976.23 0x001035C8 (8 bytes).
 *   Getter: returns the word stored at offset 0x50 of the object given as
 *   first argument.  The retail code is `jr $ra` with
 *   `lw $v0, 0x50($a0)` in its delay slot.
 *   The layout of the object is only known from the code itself: the word at
 *   offset 0x50 is read, nothing else is touched.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Boot_001035C8_Object
{
    unsigned char pad_000[0x50];
    int          field_050;
} Boot_001035C8_Object;

int func_001035C8(Boot_001035C8_Object *self)
{
    return self->field_050;
}
