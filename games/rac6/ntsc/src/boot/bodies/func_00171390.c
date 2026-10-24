/* func_00171390 -- SCUS_976.23 0x00171390, 84 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171390 (84 octets)
 *   Range -1 dans la case (table + index*4 + 0x2C4), met 2 dans les
 *   drapeaux du noeud (table + index*0x30 + 0x3C4 + 4) puis y pose le
 *   bit 31 ; rend zero.
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
    char pad[0x24];
    int index;
    char pad2[0x28];
    char *table;
};

int func_00171390(struct BootObj *object)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);
    int *slot = (int *)object->table + object->index;
    int result = 0;

    *(int *)((char *)slot + 0x2C4) = -1;
    node->flags = 2;
    node->flags |= 0x80000000u;
    return result;
}
