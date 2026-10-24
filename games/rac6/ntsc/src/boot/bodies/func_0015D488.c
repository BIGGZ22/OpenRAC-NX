/* func_0015D488 -- SCUS_976.23 0x0015D488, 128 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0015D488 (128 octets)
 *   Cadre 0x20 (slots de 16 octets, sq/lq).
 *   flag != 0 : si self->champ_00 est deja pose, on rend 0 ; sinon on appelle
 *   la fonction virtuelle avec flag et, si elle rend 0, on pose
 *   champ_00 = 1.
 *   flag == 0 : on appelle la fonction virtuelle avec 0 et, si elle rend 0,
 *   on pose champ_00 = 0.
 *   Les deux branches partagent le retour final 0.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct {
    char pad_000[0x08];
    short offset_08;
    int (*fn_0C)(void *, int);
} Vtbl_0015D488;

typedef struct {
    int field_00;
    char pad_004[0x08];
    Vtbl_0015D488 *vtbl_0C;
} Obj_0015D488;

int func_0015D488(Obj_0015D488 *self, int flag)
{
    if (flag != 0) {
        Vtbl_0015D488 *vt;
        int result;

        if (self->field_00 != 0) {
            return 0;
        }
        vt = self->vtbl_0C;
        result = vt->fn_0C((char *)self + vt->offset_08, flag);
        if (result != 0) {
            return result;
        }
        self->field_00 = 1;
    } else {
        Vtbl_0015D488 *vt = self->vtbl_0C;
        int result = vt->fn_0C((char *)self + vt->offset_08, 0);

        if (result != 0) {
            return result;
        }
        self->field_00 = 0;
    }
    return 0;
}
