/* __do_global_dtors_LEVEL19_001CABE8 -- LEVEL19.REL 0x001CABE8, 84 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Routine de liaison du compilateur EE-GCC 2.9 (crt) : parcourt la liste
 *   des destructeurs globaux.  Le curseur de la liste est un objet de
 *   donnees du module place a l'adresse absolue 0x0028B330 (mesure :
 *   lui 0x29 / lw -0x4CD0 / sw -0x4CD0) ; il est declare externe pour que
 *   le harnais le pose a cette adresse (sidecar .map).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111-sdkas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef void (*dtor_t) (void);

extern dtor_t *__dtor_list;

void
__do_global_dtors (void)
{
  while (*__dtor_list)
    {
      __dtor_list++;
      (*(__dtor_list - 1)) ();
    }
}
