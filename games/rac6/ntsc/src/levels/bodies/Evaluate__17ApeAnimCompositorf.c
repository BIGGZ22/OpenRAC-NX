/* Evaluate__17ApeAnimCompositorf -- FE.REL 0x00091D10, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La surcharge sans drapeau delegue a Evaluate__17ApeAnimCompositorfb avec
 *   le drapeau faux ($a1) et le temps recu (toujours en $f12, jamais copie).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeAnimCompositor ApeAnimCompositor;

extern float Evaluate__17ApeAnimCompositorfb(ApeAnimCompositor *self, int flag,
                                              float time);

float Evaluate__17ApeAnimCompositorf(ApeAnimCompositor *self, float time)
{
    return Evaluate__17ApeAnimCompositorfb(self, 0, time);
}
