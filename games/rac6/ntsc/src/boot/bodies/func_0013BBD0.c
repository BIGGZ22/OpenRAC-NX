/* func_0013BBD0 -- SCUS_976.23 0x0013BBD0, 76 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x0013BBD0 : asks func_143A10 for an object (written
 *   through the second argument); when it fails (v0 == 0) the object is
 *   handed to func_141E00 and the incoming floats.
 *
 *   addiu  $sp, $sp, -0x30 ; swc1 $f20, 0x20($sp)
 *   daddu  $a1, $sp, $zero ; sq $ra, 0x10($sp)
 *   jal    func_143A10       ; mov.s $f20, $f12
 *   bnez   $v0, end      ; lq $ra, 0x10($sp)
 *   lw     $a0, 0x0($sp) ; mov.s $f12, $f20
 *   jal    func_141E00
 *   lq     $ra, 0x10($sp) ; lwc1 $f20, 0x20($sp)
 *   jr     $ra             ; addiu $sp, $sp, 0x30
 *   mov.s $f13, $f21 ; the second float is passed too

 * Placement: virtual address in the boot executable.
 * Proof: the boot ASTRA route (boot-astragf:-O2 -G0) compiles this source
 * to the retail bytes of that placement, byte for byte.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_143A10(int self, int *out);
void func_141E00(int object, float value, float value2);

void func_0013BBD0(int self, float value, float value2)
{
    int out;

    if (func_143A10(self, &out) == 0)
        func_141E00(out, value, value2);
}
