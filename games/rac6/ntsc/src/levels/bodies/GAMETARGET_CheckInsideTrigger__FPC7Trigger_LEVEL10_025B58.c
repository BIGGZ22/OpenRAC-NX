/* GAMETARGET_CheckInsideTrigger__FPC7Trigger_LEVEL10_025B58 -- LEVEL10.REL 0x025B58, 32 bytes.
 *
 * Asks a trigger whether a fixed point lies inside it.
 *
 * Evidence:
 *   retail: calls func_0683D0(self, 0x31CB24) and returns.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Trigger Trigger;

void func_0683D0(Trigger *self, void *table);

extern char D_31CB24[];

void GAMETARGET_CheckInsideTrigger__FPC7Trigger(Trigger *self)
{
    func_0683D0(self, (void *)D_31CB24);
}
