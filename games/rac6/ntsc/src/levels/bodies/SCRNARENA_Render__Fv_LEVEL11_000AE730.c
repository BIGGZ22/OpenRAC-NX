/* SCRNARENA_Render__Fv_LEVEL11_000AE730 -- LEVEL11.REL 0x000AE730, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le rendu de l'ecran de l'arene est le rendu du menu de pause : l'objet
 *   tPAUSEMENU est un global du module a l'adresse 0x3C36C8 (nom retail
 *   inconnu) et son adresse est passee a PAUSEMENU_Render (0x095D90).
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

typedef struct tPAUSEMENU tPAUSEMENU;

extern void PAUSEMENU_Render__FP10tPAUSEMENU(tPAUSEMENU *menu);

/* Objet global du module a l'adresse 0x3C36C8 (nom retail inconnu). */
extern char D_003C36C8[];

void SCRNARENA_Render__Fv(void)
{
    PAUSEMENU_Render__FP10tPAUSEMENU((tPAUSEMENU *)D_003C36C8);
}
