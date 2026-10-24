/* CreateFileNameWithCurrentPath__14MemCardManagerPCcT1Pci -- FE.REL 0x026F18, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   CreateFileNameWithCurrentPath__14MemCardManagerPCcT1Pci
 *   @ FE.REL 0x026F18 (64 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct MemCardManager MemCardManager;

extern void Snprintf__9ApeStringPciPCce(char *dest, int size, const char *format, ...);
extern const char D_0011B310[];

char *CreateFileNameWithCurrentPath(MemCardManager *self, const char *path,
                                    char *dest, int size)
{
    Snprintf__9ApeStringPciPCce(dest, size, D_0011B310, self, path);
    return dest;
}
