/* from src/core/001232F0.c: handwritten assembly, no C candidate (evidence only) */
/*
 * Not a C target, and the row is not even a whole function. Two independent
 * signs:
 *
 * 1. The first word, at 0x00124414, is `pref 0x0D, -0x3233($t6)`: it reads $t6,
 *    which nothing in the row sets, and the body's real work starts at +0x4
 *    (`lui $v0,0x1000`). A word that only makes sense after somebody else's
 *    prologue is the split-off piece of a neighbour (the previous row ends mid
 *    sequence), so even the boundary is wrong.
 *
 * 2. From +0x2C the body is COP2/VU0 register plumbing that no compiler emits:
 *
 *      +0x2C  sync
 *      +0x34  cfc2 $a0, $28      +0x38 nop            +0x3C ori $a0,$a0,0x200
 *      +0x40  ctc2 $a0, $28      +0x44 sync
 *      +0x48  cfc2 $a0, $29      ... poll of $a0 & 0x100 with bnez/loop ...
 *      +0x68  ctc2 $a0, $404
 *      +0x78  0x78460000 qmfc2   +0x8C 0x7CA60000 qmtc2
 *      +0x98  0x78620000 qmfc2   +0x9C 0x7CA20000 qmtc2
 *
 *    (rabbitizer prints the four 0x78/0x7C words as INVALID: they are the
 *    EE's qmfc2/qmtc2, macro moves between a GPR and a VU0 register.) $28/$29
 *    are the COP2 control registers this unit uses for its DMA handshake, and
 *    the routine ends with `jr $ra` / `nop` after `sw $a3,0x0($a0)`.
 *
 * This is an ASM_FUNC (handwritten) on the evidence above, not a near miss: it
 * is also a row to join to its neighbour, or to re-bound, in tools/levels.py.
 * No C is recorded.
 */
