/* PAUSEMENU_SetTextUseSubnodeColor__FP10tPAUSEMENUUiUib -- FE.REL 0x0352D0, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   PAUSEMENU_SetTextUseSubnodeColor__FP10tPAUSEMENUUiUib
 *   @ FE.REL 0x0352D0 (64 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tPAUSEMENU_Node tPAUSEMENU_Node;
typedef struct tPAUSEMENU tPAUSEMENU;

struct tPAUSEMENU_Node {
    unsigned char pad0[0x24];
    unsigned char useSubnodeColor;
    unsigned char pad1[11];
};

struct tPAUSEMENU {
    tPAUSEMENU_Node **nodes;
    unsigned int count;
};

int PAUSEMENU_SetTextUseSubnodeColor(tPAUSEMENU *self, unsigned int index,
                                      unsigned int subnode, int useSubnodeColor)
{
    tPAUSEMENU_Node *node;
    if (index >= self->count)
        return 0;
    node = self->nodes[index];
    node = node + subnode;
    node->useSubnodeColor = useSubnodeColor;
    return 1;
}
