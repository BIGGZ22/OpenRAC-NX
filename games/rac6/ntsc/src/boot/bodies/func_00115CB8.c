/* func_00115CB8 -- SCUS_976.23 0x00115CB8, 28 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00115CB8 (28 octets)
 *   Transfert direct vers func_1158D0.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_1158D0(void *object, int flags);

void func_00115CB8(void *object, int flags)
{
    func_1158D0(object, flags);
}
