/* func_0013ADB0 -- SCUS_976.23 0x0013ADB0, 60 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x0013ADB0 : asks func_15B460 for an object (written
 *   through the second argument); when it fails (v0 == 0) the object is
 *   handed to func_15C720.
 *
 *   addiu  $sp, $sp, -0x30 ; swc1 $f20, 0x20($sp)
 *   daddu  $a1, $sp, $zero ; sq $ra, 0x10($sp)
 *   jal    func_15B460       ; mov.s $f20, $f12
 *   bnez   $v0, end      ; lq $ra, 0x10($sp)
 *   lw     $a0, 0x0($sp) ; mov.s $f12, $f20
 *   jal    func_15C720
 *   lq     $ra, 0x10($sp) ; lwc1 $f20, 0x20($sp)
 *   jr     $ra             ; addiu $sp, $sp, 0x30

 * Placement: virtual address in the boot executable.
 * Proof: the boot ASTRA route (boot-astragf:-O2 -G0) compiles this source
 * to the retail bytes of that placement, byte for byte.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_15B460(int self, int *out);
void func_15C720(int object, float value);

void func_0013ADB0(int self, float value)
{
    int out;

    if (func_15B460(self, &out) == 0)
        func_15C720(out, value);
}
