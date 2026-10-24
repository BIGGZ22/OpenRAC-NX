/* func_001713E8 -- SCUS_976.23 0x001713E8, 272 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001713E8 (272 octets)
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
    char pad0[0x40];
    float f40;
    char pad1[8];
    float f4c;
};

struct BootBig
{
    char pad00[0x90];
    struct BootInner *f90;
    char pad94[0x0C];
    float fa0;
    char padA4[0x9C];
    float f140;
    char pad144[0x28];
    float f16c;
    char pad170[0x18];
    float f188;
    char pad18c[0x0C];
    float f198;
};

struct BootLeaf
{
    char pad[0x38];
    float f38;
};

struct BootMid
{
    char pad[0x110];
    struct BootLeaf *f110;
};

struct BootObj
{
    char pad00[4];
    int f04;
    char pad08[4];
    struct BootBig *f0c;
    char pad10[4];
    struct BootMid *f14;
    char pad18[0x0C];
    int index;
    char pad28[0x08];
    int f30;
    char pad34[0x1C];
    char *table;
    float f54;
    float f58;
};

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

int func_001713E8(struct BootObj *object, float scale)
{
    struct BootNode *node = (struct BootNode *)((char *)object->table
                                                + 0x3C4
                                                + object->index * 0x30);
    int result = 0;

    if (object->f0c) {
        float *slot = (float *)object->f0c + object->f04;

        scale *= 1.0f - object->f0c->f188;
        scale *= 1.0f - object->f0c->f198;
        scale *= object->f0c->f90->f4c;
        scale *= object->f0c->f140;
        scale *= object->f0c->f16c;
        scale *= object->f0c->f90->f40;
        scale *= slot[0x3E];
        scale *= object->f0c->fa0;
    }
    if (object->f14 && object->f14->f110)
        scale *= object->f14->f110->f38;
    object->f54 = scale;
    node->f10 = (int)(scale * (1.0f - object->f58) * 16384.0f);
    node->f14 = (int)(object->f54 * object->f58 * 16384.0f);
    node->flags |= 0x4u;
    return result;
}
