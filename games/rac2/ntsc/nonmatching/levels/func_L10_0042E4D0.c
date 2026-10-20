/* func_L10_0042E4D0 (192 bytes, 1 place) - TI128 destination wall, ASM_FUNC candidate.
 *
 * Readable: func_2FA1B0(a0+0x10, 16, 0.8f, *(void**)(a0+0x68)) decides the arm; two of the
 * three arms copy a 128-bit object through $v0 (`lq $v0,0x0($v1)` 0x78620000 then
 * `sq $v0,0x0($sp)` 0x7FA20000, or `lq $v0,0x0($sp)` 0x7BA20000 then
 * `sq $zero,0x0($s0)` 0x7E020000), and each arm then calls func_308E10 with a different
 * float constant (0x3F4CCCCD, 0x3E888889, 0x3D088889).
 * The wall is the same one worker 02 reconfirmed three times: this cc1 folds the constant
 * destination displacement into the `sq` where retail materialises the pointer with an
 * `addiu` first, so the 128-bit copies come out one instruction short each (retail's
 * `addiu $v1,$v1,0xC0` / `addiu $v1,$v1,-0x500` around the lq/sq pair).  Not retried here
 * beyond reading it, per that finding.
 */
