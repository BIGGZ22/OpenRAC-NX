/* LEVEL_RequestRestart__Fv_LEVEL10_01EF78 -- LEVEL10.REL 0x01EF78, 32 bytes.
 *
 * Asks the game-mode layer for a restart (mode 8).
 *
 * Evidence:
 *   retail: calls func_025E58(8, 0) and returns.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

void func_025E58(int mode, int value);

void LEVEL_RequestRestart__Fv(void)
{
    func_025E58(8, 0);
}
