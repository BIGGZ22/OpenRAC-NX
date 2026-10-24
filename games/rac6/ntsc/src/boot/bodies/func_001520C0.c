/* func_001520C0 -- SCUS_976.23 0x001520C0, 128 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x001520C0 : dispatches through the object's table, then flags the state.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef int (*Boot_001520C0_Fn)(void *self, int arg);

typedef struct Boot_001520C0_Table
{
    unsigned char        pad_008[0x08];
    short                offset;
    unsigned char        pad_00A[0x02];
    Boot_001520C0_Fn     fn;
} Boot_001520C0_Table;

typedef struct Boot_001520C0_Object
{
    unsigned char        pad_00C[0x0C];
    int                  state;
    unsigned char        pad_130[0x130 - 0x10];
    Boot_001520C0_Table *table;
} Boot_001520C0_Object;

int func_001520C0(Boot_001520C0_Object *self, int flag)
{
    int result;

    if (flag) {
        if (self->state != 0) {
            return 0;
        }
        result = self->table->fn((char *)self + self->table->offset, flag);
        if (result != 0) {
            return result;
        }
        self->state = 1;
    } else {
        result = self->table->fn((char *)self + self->table->offset, 0);
        if (result != 0) {
            return result;
        }
        self->state = 0;
    }
    return 0;
}
