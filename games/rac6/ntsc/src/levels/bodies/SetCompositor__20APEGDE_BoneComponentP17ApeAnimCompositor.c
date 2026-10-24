/* SetCompositor__20APEGDE_BoneComponentP17ApeAnimCompositor -- LEVEL13.REL 0x04B988, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   (8 octets)
 *   La methode ecrit le compositeur dans le champ a l'offset 0x0C du composant
 *   et rend la main : aucune pile n'est reservee.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct APEGDE_BoneComponent APEGDE_BoneComponent;
typedef struct ApeAnimCompositor ApeAnimCompositor;

struct APEGDE_BoneComponent {
    unsigned char unknown_000[0x0C];
    ApeAnimCompositor *compositor;
};

void SetCompositor__20APEGDE_BoneComponentP17ApeAnimCompositor(
    APEGDE_BoneComponent *self, ApeAnimCompositor *compositor)
{
    self->compositor = compositor;
}
