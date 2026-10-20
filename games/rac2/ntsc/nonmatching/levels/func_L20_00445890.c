/* func_L20_00445890 (568 bytes, level 20): NOT ATTEMPTED in this wave; the analysis is
 * recorded for the next attempt.
 *
 * The body is recovered from build/20_yeedil/asm/5B0B0.s:
 *  - the state is arg0[0x68]; if it is null the function returns;
 *  - `func_00283B98(state[0x20], 0.10471977f)` and `func_00283198(...)` with the state
 *    written back, then `* 4.0f - 3.0f`, then `* 128.0f` and func_00283CF0;
 *  - the result is clamped with `slti`/`movz` to 0x1F or 0x20 - the `movz` is what a
 *    ternary or a conditional assignment produces;
 *  - arg0[0x20] selects: 0 and 0xFE/0xFD go one way, 1 and 2 another (func_0029FCC0 and
 *    func_00282E80 with the boot table D_189EA0), 3 and 4 the third (func_002A0430);
 *  - the table at D_L20_001B2F9C is walked, a byte is set at +0xBC, and
 *    func_002A0F48 and func_L20_002C5A48 are called;
 *  - the return value is built with `or`/`sll` and stored at arg0[0x78].
 *
 * The walls: the `movz` clamp and the branch-likely loop (`beql`/`bgezl`), plus the exact
 * order of the two `or`/`sll` pairs at the end. The unit has to be the -G8 sibling (it
 * reads gp+0x3FAC).
 */
