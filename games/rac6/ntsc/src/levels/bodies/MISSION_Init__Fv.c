/* MISSION_Init__Fv -- LEVEL11.REL 0x00028B28, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'initialisation de la mission ne fait que mettre a jour les etats des
 *   missions ; la fonction appelee est MISSION_UpdateMissionStates__Fv
 *   (0x028D00), qui n'a pas d'argument.
 *   Preuve : chaine gnu-ee-20261015:-O2 -G0 (egalement exacte sur
 *   gnu-ee-trial-20261015:-O2 -G0), comparaison octet pour octet au retail
 *   au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void MISSION_UpdateMissionStates__Fv(void);

void MISSION_Init__Fv(void)
{
    MISSION_UpdateMissionStates__Fv();
}
