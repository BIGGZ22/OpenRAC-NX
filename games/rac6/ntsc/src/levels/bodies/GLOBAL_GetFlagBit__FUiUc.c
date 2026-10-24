/* GLOBAL_GetFlagBit__FUiUc -- FE.REL 0x008F00, 32 bytes.
 *
 * Reads one bit of the global flag array.
 *
 * Evidence:
 *   Le pointeur global (adresse module 0xF4D8) designe un bloc dont le
 *   tableau d'octets de drapeaux commence a l'offset 0x4E0 ; l'octet lu est
 *   masque par le bit demande.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GLOBALVARS_s GLOBALVARS_s;

struct GLOBALVARS_s {
    unsigned char pad[0x4E0];
    unsigned char flags[0x100];
};

extern GLOBALVARS_s *GLOBALVARS_pGlobals;

unsigned int GLOBAL_GetFlagBit__FUiUc(unsigned int index, unsigned char bit)
{
    return GLOBALVARS_pGlobals->flags[index] & (1u << bit);
}
