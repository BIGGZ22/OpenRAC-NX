/* func_L09_003F9970 (256 bytes, 2 places) - four MMI/INVALID words plus five calls; the
 * biggest INVALID count of this tour.  Classified by pattern only, no C measured.
 *
 * Same triage as the L02 rows: identify whether the four `.word # INVALID` entries are
 * lq/sq (mode(TI) may reach them) or the 0x7000_xxxx MMI family (out of reach without a
 * compiler patch).  The row is otherwise ordinary - no max.s/min.s, no $gp.
 */
