/* func_00171748 -- SCUS_976.23 0x00171748, 168 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171748 (168 octets)
 *   Bascule un bit de drapeau du noeud selon le signe du champ +8 du
 *   second argument : si ce champ est < -9999 on passe de 0x100 a 0x200,
 *   sinon l'inverse ; puis appelle func_172A70 (objet, argument).
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

struct BootArg
{
    char pad[8];
    int f8;
};

extern void func_172A70(struct BootObj *object, struct BootArg *arg);

void func_00171748(struct BootObj *object, struct BootArg *arg)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);

    if (arg->f8 < -9999) {
        node->flags &= ~0x100u;
        node->flags |= 0x200u;
    } else {
        node->flags &= ~0x200u;
        node->flags |= 0x100u;
    }
    func_172A70(object, arg);
}
