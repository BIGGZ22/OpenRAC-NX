/* OperaController_IntroMovieFinishedCB__FPv -- LEVEL18.REL 0x0CBD78, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail : jr $ra ; sb $zero, 0x45($a0)
 *   L'octet "film d'intro fini" est a l'offset 0x45 de l'objet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct OperaController {
    unsigned char field_00[0x45];
    unsigned char intro_movie_finished;
} OperaController;

void OperaController_IntroMovieFinishedCB__FPv(void *self)
{
    ((OperaController *)self)->intro_movie_finished = 0;
}
