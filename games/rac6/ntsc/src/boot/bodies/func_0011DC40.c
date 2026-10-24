/* func_0011DC40 -- SCUS_976.23 0x0011DC40, 32 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0011DC40 (32 octets)
 *   Rend table[index] si self != 0, sinon 0.
 *   Table : pointeur stocke a self+0x250.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct {
    char pad_000[0x250];
    int *table_250;
} Obj_0011DC40;

int func_0011DC40(Obj_0011DC40 *self, int index)
{
    if (self != 0) {
        return self->table_250[index];
    }
    return 0;
}
