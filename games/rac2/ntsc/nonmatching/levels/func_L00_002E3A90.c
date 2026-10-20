/* Not C in its present form: the accumulate block from +0x54 to +0x94 is R5900/MMI-class.
 * The first ten instructions are ordinary C (a flag load and two bound tests over the
 * arguments), and the tail is an ordinary nine-iteration walk over four-byte records; but the
 * block itself carries three-operand `mult` words (0x00C31818, 0x00852818), `mtlo $zero`
 * (0x00000013), `mflo`, and the word 0x70820000 that rabbitizer prints `.word # INVALID`
 * (opcode 0x1C, MMI-class), so no plain-C spelling reaches the bytes.
 *
 * What would unblock it: ASM_FUNC for the whole row, or a C spelling for the MMI accumulate
 * that no source form of this cc1 emitted so far.
 */
