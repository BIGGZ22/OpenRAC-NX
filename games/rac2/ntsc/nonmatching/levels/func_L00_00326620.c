/* Not attempted: same quad-copy class as nonmatching/levels/func_L18_003FC9B8.c - 0x78A20000
 * and 0x78620000 (LQ) into 0x7CC20000 and 0x7C820000 (SQ), copying two sixteen-byte blocks
 * between the object and the newly created one - inside an else-branch that also runs two FP
 * divides and reuses the same helper callees as its neighbours.
 *
 * What would unblock it: the shared quad-copy plan, then an ordinary C body around it.
 */
