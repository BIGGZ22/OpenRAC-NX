/* LEVEL18_017B00 -- LEVEL18.REL 0x017B00, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La fonction appelle func_14F558 (MU_UnderdampedSpringSetup) avec
 *   l'adresse d'un objet global (0x258D68) et trois constantes flottantes :
 *   0x427B53D1 = 62.831853 (20 * pi)
 *   0x41A00000 = 20.0
 *   0x3D088889 = 0.0333333351 (1/30)
 *   Les constantes sont chargees par lui/ori puis mtc1 dans $f12, $f13, $f14.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_14F558(void *spring, float k, float d, float c);

extern char g_springCoeff[];   /* objet global a 0x258D68 */

void func_017B00(void)
{
    func_14F558(g_springCoeff, 62.831853f, 20.0f, 0.0333333351f);
}
