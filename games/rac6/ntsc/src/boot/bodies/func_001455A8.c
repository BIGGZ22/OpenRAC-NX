/* func_001455A8 -- SCUS_976.23 0x001455A8, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001455A8 (64 octets)
 *   Cadre 0x20 (slot de 16 octets, sq/lq).
 *   func_148778(self, &local) : si le retour n'est pas nul, on sort ;
 *   sinon appel virtuel base->sous_18->fn(base + offset, 1).
 *   fn renvoie un int (le registre $v0 est ainsi reserve par l'appelant).
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
    char pad_000[0x18];
    void *sub_18;
} Base_001455A8;

typedef struct {
    char pad_000[0x30];
    short offset_30;
    int (*fn_34)(void *, int);
} Vtbl_001455A8;

extern int func_148778(void *self, int *out);

void func_001455A8(void *self)
{
    int local;

    if (func_148778(self, &local) != 0) {
        return;
    }
    {
        Base_001455A8 *base = (Base_001455A8 *)local;
        Vtbl_001455A8 *vt = (Vtbl_001455A8 *)base->sub_18;

        vt->fn_34((char *)base + vt->offset_30, 1);
    }
}
