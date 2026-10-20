/* func_L20_00441CB8 (1160 bytes, level 20): NOT ATTEMPTED in this wave; the blocker is
 * measured and recorded.
 *
 * Four 128-bit moves, two of them with a constant offset on either side:
 *   `lq $v0,0x0($s2) ; sq $v0,0x0($a0)` (both plain registers: $s2 and $a0 are parameters,
 *   this one is fine) and
 *   `addiu $a0,$s0,0x17E0 ; addiu $v1,$s1,0x10 ; lq $v0,0x0($a0) ; sq $v0,0x0($v1)`.
 * The second one has the constant offset on the SOURCE as well as on the destination, and
 * this cc1 folds both into the `lq`/`sq`. The other two pairs of the body are the same
 * shape.
 *
 * The one escape of that wall that exists (measured in this wave, see
 * nonmatching/levels/func_L20_004499E8.c) is a volatile pointer OBJECT: it reaches the
 * retail's zero-displacement `lq`/`sq` at the price of a stack store and a reload per
 * pointer - eight bytes over each.
 */
