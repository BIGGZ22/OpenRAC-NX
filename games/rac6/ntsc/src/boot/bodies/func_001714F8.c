/* func_001714F8 -- SCUS_976.23 0x001714F8, 96 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001714F8 (96 octets)
 *   Le second parametre est un flottant (registre $f12) qui sert
 *   d'accumulateur : si l'objet a un sous-objet en +0x0C, on le multiplie
 *   par son champ +0x144 puis par le champ +0x58 de son sous-champ +0x90 ;
 *   le resultat est converti en entier et range dans le champ +0x18 du
 *   noeud, dont on pose aussi le bit 3 des drapeaux.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+ps2eeas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct BootInner
{
    char pad[0x58];
    float f58;
};

struct BootBig
{
    char pad[0x90];
    struct BootInner *f90;
    char pad2[0xB0];
    float f144;
};

struct BootObj
{
    char pad[0x0C];
    struct BootBig *f0c;
    char pad2[0x14];
    int index;
    char pad3[0x28];
    char *table;
};

struct __attribute__((packed)) BootNode
{
    char pad00[4];
    unsigned int flags;
    char pad08[0x10];
    int f18;
    char pad1c[8];
    unsigned int f24;
    char pad28[8];
};

int func_001714F8(struct BootObj *object, float scale)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);
    int result = 0;

    if (object->f0c) {
        scale *= object->f0c->f144;
        scale *= object->f0c->f90->f58;
    }
    node->f18 = (int)scale;
    node->flags |= 0x8u;
    return result;
}
