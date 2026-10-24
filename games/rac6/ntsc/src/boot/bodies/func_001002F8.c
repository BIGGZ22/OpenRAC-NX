/* func_001002F8 -- SCUS_976.23 0x001002F8, 12 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001002F8 (12 octets)
 *   Ecrit l'argument dans le global 0x0019EF0C.
 *   lui $v0,0x1A ; jr $ra ; sw $a0,-0x10F4($v0) ; nop
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int g_boot_0019EF0C;

void func_001002F8(int value)
{
    g_boot_0019EF0C = value;
}
