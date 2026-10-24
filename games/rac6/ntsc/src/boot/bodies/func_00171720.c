/* func_00171720 -- SCUS_976.23 0x00171720, 40 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171720 (40 octets)
 *   Lit l'entier +0x2C4 de l'element (base + index*4 + 0x2C4) et rend
 *   son bit de signe inverse dans *out ; rend zero.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct BootObj
{
    char pad[0x24];
    int index;
    char pad2[0x28];
    int *field50;
};

int func_00171720(struct BootObj *object, int *out)
{
    int result = 0;
    int *slot = object->field50 + object->index + 0xB1;

    *out = (*slot >= 0);
    return result;
}
