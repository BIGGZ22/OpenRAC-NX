/* DrawVLINE__FP6VLINES_LEVEL11_000630A8 -- LEVEL11.REL 0x000630A8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le trait est dessine par DrawLines : le meme bloc de sommets sert
 *   d'argument 1, le mode 2 est constant et le nombre de segments vient du
 *   champ situe a l'offset 0xC du bloc VLINES.
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

typedef struct PUBE_VERT PUBE_VERT;

typedef struct VLINES {
    unsigned int reserved_000;
    unsigned int reserved_004;
    unsigned int reserved_008;
    unsigned int segment_count;
} VLINES;

extern void DrawLines__FPC9PUBE_VERTUiUi(PUBE_VERT *verts,
                                         unsigned int mode,
                                         unsigned int segments);

void DrawVLINE__FP6VLINES(VLINES *lines)
{
    DrawLines__FPC9PUBE_VERTUiUi((PUBE_VERT *)lines, 2, lines->segment_count);
}
