/* BeginLoad__12ApeLevelInst -- FE.REL 0x00002870, 44 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La methode memorise dans son premier mot le numero de monde rendu par
 *   l'entree statique ApeWorld::BeginLoad(1) (son $a0 est l'identifiant :
 *   la fonction retail ecrase $a1 par zero et appelle l'alias du monde).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeLevelInst {
    int worldIndex;
} ApeLevelInst;

extern int BeginLoad__8ApeWorldUi(unsigned int worldId);

void BeginLoad__12ApeLevelInst(ApeLevelInst *self)
{
    self->worldIndex = BeginLoad__8ApeWorldUi(1);
}
