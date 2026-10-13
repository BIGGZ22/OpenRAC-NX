/* NEAR MISS — 112 bytes emitted, 116 expected (one instruction short).
 * last check.py line: "14_aranos_prison: 112 bytes, expected 116" / "func_L14_00426DE8: 0 of 1 places match"
 *
 * Everything up to the first call matches word for word. The single missing word is the
 * retail's write-back + argument copy
 *     sll   $s0,$s0,24        ; v = v << 24   (in the jal delay slot)
 *     daddu $t0,$s0,$zero     ; arg5 = v
 * where this compiler forwards the shift into the argument register instead:
 *     sll   $t0,$s0,24
 * (v is dead after the call, so gcc copy-propagates it). The data base is also materialised
 * slightly differently (retail folds %hi/%lo into one register and uses two offset loads;
 * ours kept the addend in a second register) but the instruction count there is the same.
 *
 * Tried: `v <<= 24; f(..., v);` and `int w = v << 24; f(..., w);` — both forward.
 */
void func_L14_00426DE8(char *a0)
{
    char *v0;
    int v;

    v0 = *(char **)(a0 + 0x68);
    v = (int)((1.0f - *(float *)(v0 + 0x68)) * 128.0f);
    func_385CB8(0x42, 9);
    v <<= 24;
    func_2EF6D8(0, *(short *)(D_001A6480 + 0x152), 0,
                *(short *)(D_001A6480 + 0x150), v);
}
