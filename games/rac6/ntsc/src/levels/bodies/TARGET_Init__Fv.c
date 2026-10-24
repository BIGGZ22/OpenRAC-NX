/* TARGET_Init__Fv -- LEVEL14.REL 0x025160, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   64 octets.
 *   Le pointeur de boucle est une variable d'induction : il vit dans $a1 et
 *   avance de 0x64 (taille d'une entree) par tour.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct TARGET_ENTRY TARGET_ENTRY;

struct TARGET_ENTRY {
    unsigned char reserved_000[0x5C];
    unsigned int field_05C;
    unsigned int field_060;
};

extern TARGET_ENTRY TARGET_entries[];
extern unsigned int TARGET_count;
extern void *TARGET_current;

void TARGET_Init__Fv(void)
{
    unsigned int i;

    TARGET_current = TARGET_entries;
    TARGET_count = 0;
    for (i = 0; i < 16; i++)
        TARGET_entries[i].field_05C = 0;}
