/* func_0013B200 -- SCUS_976.23 0x0013B200, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 @ 0x0013B200 - 64 bytes
 *   addiu  $sp, $sp, -0x30 ; swc1 $f20, 0x20($sp)
 *   daddu  $a1, $sp, $zero ; sq $ra, 0x10($sp)
 *   jal    func_143A10      ; mov.s $f20, $f12
 *   bnez   $v0, end         ; lq $ra, 0x10($sp)
 *   lw     $a0, 0x0($sp) ; mov.s $f12, $f20
 *   jal    func_140FC0      ; daddu $a1, $zero, $zero
 *   lq     $ra, 0x10($sp) ; lwc1 $f20, 0x20($sp)
 *   jr     $ra              ; addiu $sp, $sp, 0x30
 *   Authenticator: asks func_143A10 for an object (written through the second
 *   argument); when it fails (v0 == 0) the object is handed to func_140FC0
 *   together with the incoming float.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_143A10(int self, int *out);
void func_140FC0(int object, int flag, float value);

void func_0013B200(int self, float value)
{
    int out;

    if (func_143A10(self, &out) == 0)
        func_140FC0(out, 0, value);
}
