/* HUD_DrawHangulJamoScaled__FUcffffUi -- LEVEL11.REL 0x0007F620, 336 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ...
 *   Le glyphe hangul est dessine par HUD_DrawTextureSimpleArea : la texture
 *   vient de GetFontTextureID(1), les quatre dimensions sont les arguments
 *   flottants du rendu et les quatre coordonnees les quatre "unsigned short"
 *   (les "short" du nom retail sont les quatre premiers).  La quatrieme
 *   coordonnee vaut 0xFFFFFFFF.
 *   La table des jamo (4 octets par caractere) commence a 0x270468 ; l'octet
 *   b3 donne la hauteur, b0 et b2 la position horizontale, b1 la largeur.
 *   Preuve : chaine gnu-ee-trial-20261015:-O2 -G0, comparaison octet pour
 *   octet au retail au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef unsigned char Uc;
typedef unsigned int Ui;

extern void *GetFontTextureID__14DrawObjManagerUi(Ui index);
extern void HUD_DrawTextureSimpleArea__FP10AglTexturessssUsUsUsUsUi(
    void *texture,
    short a, short b, short c, short d,
    unsigned short e, unsigned short f, unsigned short g, unsigned short h,
    Ui i);
extern Ui fptoui(float x);

/* Table des jamo du module a l'adresse 0x270468 (nom retail inconnu). */
extern unsigned char D_00270468[];

Ui HUD_DrawHangulJamoScaled__FUcffffUi(Uc code, float f1, float f2, float f3,
                                       float f4, Ui flag)
{
    unsigned char *row = D_00270468 + (((code - 0x31) & 0xFF) * 4);
    float sc = f4 * 16.0f;
    float v22 = f3 + f3;
    float v23 = f1;
    float v20 = f2;
    float v21 = (float)row[3];
    int a5, a6;
    short sc16;
    void *tex;

    a5 = row[0] * 16;
    a6 = row[1] * 16;
    sc16 = (short)sc;
    v21 = v21 * f3;
    tex = GetFontTextureID__14DrawObjManagerUi(1);
    v20 = v20 + v22;
    a5 = a5 + row[2];

    HUD_DrawTextureSimpleArea__FP10AglTexturessssUsUsUsUsUi(
        tex,
        (short)v23,
        (short)(v20 - (float)sc16),
        (short)v20,
        (short)a5,
        (unsigned short)a6,
        (unsigned short)(row[3] + a5),
        (unsigned short)(a6 + 16),
        (unsigned short)flag,
        0xFFFFFFFFu);

    return fptoui(v21);
}
