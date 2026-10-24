/* func_0013A8E0 -- SCUS_976.23 0x0013A8E0, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x0013A8E0 : queries func_15B460, then hands the result to func_15C610.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_15B460(void *self, void *out);
extern int func_15C610(void *out, void *arg, int flag);

int func_0013A8E0(void *self, void *arg)
{
    void *out;
    void *saved = arg;
    int result = func_15B460(self, &out);

    if (result == 0) {
        result = func_15C610(out, saved, 0);
    }
    return result;
}
