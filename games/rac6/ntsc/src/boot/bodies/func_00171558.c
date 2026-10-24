/* func_00171558 -- SCUS_976.23 0x00171558, 152 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171558 (152 octets)
 *   Le second parametre est un flottant ($f12) : on le transforme en
 *   (v + 1) * 0.5, on le range dans le champ +0x58 de l'objet, puis on
 *   range dans le noeud deux entiers : le champ +0x54 multiplie par
 *   (1 - v) puis par 10000, et le champ +0x54 multiplie par le champ
 *   +0x58 puis par 10000 ; on pose enfin le bit 2 des drapeaux.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+ps2eeas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct __attribute__((packed)) BootNode
{
    char pad00[4];
    unsigned int flags;
    char pad08[8];
    int f10;
    int f14;
    int f18;
    char pad1c[8];
    unsigned int f24;
    char pad28[8];
};

struct BootObj
{
    char pad[0x24];
    int index;
    char pad2[0x28];
    char *table;
    float f54;
    float f58;
};

int func_00171558(struct BootObj *object, float scale)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);
    int result = 0;

    scale = (scale + 1.0f) * 0.5f;
    object->f58 = scale;
    node->f10 = (int)(object->f54 * (1.0f - scale) * 16384.0f);
    node->f14 = (int)(object->f54 * object->f58 * 16384.0f);
    node->flags |= 0x4u;
    return result;
}
