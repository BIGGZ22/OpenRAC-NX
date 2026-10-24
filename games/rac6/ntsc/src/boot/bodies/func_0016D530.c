/* func_0016D530 -- SCUS_976.23 0x0016D530, 12 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 @ 0x0016D530 - 12 bytes (bounds body_end 0x0016D53C)
 *   sw    $a1, 0x2A4($a0)
 *   jr    $ra
 *   daddu $v0, $zero, $zero
 *   Authenticator: stores the second argument at offset 0x2A4 of the first,
 *   then returns 0 (the return value rides in the delay slot of the jr).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_0016D530(void *self, void *value)
{
    *(void **)((char *)self + 0x2A4) = value;
    return 0;
}
