/* func_00171300 -- SCUS_976.23 0x00171300, 144 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171300 (144 octets)
 *   Noeud = table + index*0x30 + 0x3C4 ; les champs 32 bits du noeud sont
 *   en acces non aligne (lwl/lwr/swl/swr) : le noeud est un struct packe,
 *   donc l'alignement de ses membres est 1.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct __attribute__((packed)) BootNode
{
    char pad00[4];
    unsigned int flags;
    char pad08[4];
    unsigned int f0c;
    char pad10[0x14];
    unsigned int f24;
    char pad28[8];
};

struct BootObj
{
    char pad[0x1C];
    int f1c;
    char pad2[0x04];
    int index;
    char pad3[0x08];
    int f30;
    char pad4[0x1C];
    char *table;
};

int func_00171300(struct BootObj *object)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);
    int result = 0;

    node->flags &= ~2u;
    node->flags |= 1u;
    node->flags |= 0x80000000u;
    if ((object->f1c & 6) && object->f30)
        node->f24 = 1;
    return result;
}
