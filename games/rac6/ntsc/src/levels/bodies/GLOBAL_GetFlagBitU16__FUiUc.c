/* GLOBAL_GetFlagBitU16__FUiUc -- LEVEL13.REL 0x035D18, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Variante 16 bits : le drapeau est lu sur un demi-mot.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GLOBALVARS GLOBALVARS;

struct GLOBALVARS {
    unsigned char unknown_000[0x4E0];
    unsigned char flags[1];
};

extern GLOBALVARS *pGV;

unsigned int GLOBAL_GetFlagBitU16__FUiUc(unsigned int index, unsigned char bit)
{
    return *(unsigned short *)(pGV->flags + index) & (1u << bit);
}
