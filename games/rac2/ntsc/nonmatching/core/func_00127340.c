/* from src/core/00127040.c: not attempted; the row opens with two words that
 * belong to no function in it.
 *
 * The row's first two instructions are `andi $v0,$v0,0x1` and
 * `pref 0x0D,-0x3233($t6)` - both read registers this row never writes, and
 * the `pref` operand is arbitrary data, not an address. The real prologue
 * (`addiu $sp,$sp,-0xB0` with 16-byte `sd` saves) starts at +0x8, and the 436
 * bytes from there read as an ordinary unit: the state word at gp-0x7E6C,
 * func_0011AC20/func_0011AC60/func_0011CF78/func_00127B88, a 0xFFFFF-
 * iteration delay loop, and func_0011D450 with 0x80000400.
 *
 * Same class as func_00127218 next door, whose eight-byte remnant also
 * belongs to the *previous* function: a tools/levels.py split, not a C body.
 * A C candidate for the 436 bytes cannot be checked against a 444-byte row.
 */
