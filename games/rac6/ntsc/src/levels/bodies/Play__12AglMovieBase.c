/* Play__12AglMovieBase -- FE.REL 0x003D18, 8 bytes.
 *
 * Empty method of the movie base object.
 *
 * Evidence:
 *   La routine ne fait rien et ne rend aucune valeur exploitee :
 *   le symbole retail est une methode de AglMovieBase.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglMovieBase AglMovieBase;

void Play(AglMovieBase *self)
{
    (void)self;
}
