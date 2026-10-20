/* func_L10_00424448 (224 bytes, 1 place) - decoded, no C measured this wave.
 *
 * One-shot init guarded by the $gp word at 0x001B13A4: sw $zero to 0x00221CFC, then the
 * flag, then func_342218(), func_37CFD8(), func_37BD00(0), func_37BCA0() and func_37BCB0(),
 * the last answering the entry count in $s0/$s1; a countdown loop over 4-byte entries
 * calling func_37D048(*p, 1) whenever *(s16*)(*p + 0xAA) equals 0xF8E (the bnel carries the
 * decrement in its annulled delay slot); then func_425798(), func_425410(0x32, 0xC),
 * func_37BCB8(0xF8E), func_37D168(v0) and, only when that answers zero,
 * func_423DC8(a0, 2).  The two jal/nop pairs and the bnez that skips the last call are
 * ordinary; the row simply was not attempted in this wave.
 */
