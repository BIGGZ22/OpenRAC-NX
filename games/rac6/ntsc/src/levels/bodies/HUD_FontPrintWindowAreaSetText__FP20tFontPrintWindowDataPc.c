/* HUD_FontPrintWindowAreaSetText__FP20tFontPrintWindowDataPc -- LEVEL14.REL 0x0841C8, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   8 octets.
 *   L'ecriture du texte est dans l'emplacement de delai du `jr $ra`.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tFontPrintWindowData {
    unsigned int reserved_00;
    unsigned int reserved_04;
    unsigned int reserved_08;
    unsigned int reserved_0C;
    unsigned int reserved_10;
    const char *text; /* +0x14 */
} tFontPrintWindowData;

void HUD_FontPrintWindowAreaSetText__FP20tFontPrintWindowDataPc(
    tFontPrintWindowData *self, const char *text)
{
    self->text = text;
}
