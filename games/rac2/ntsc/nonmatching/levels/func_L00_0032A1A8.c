/* Not attempted: quad-copy class (0x78620000 LQ into 0x7FA20000 SQ over a stack vector, plus
 * the same pair over the object) with FP moves and three helper calls around it - the same
 * blocker as nonmatching/levels/func_L18_003FC9B8.c. The stack vector at $sp+0x10 and the
 * `mov.s`/`swc1` save-restore are ordinary C once the quads have a spelling.
 *
 * What would unblock it: the shared quad-copy plan.
 */
