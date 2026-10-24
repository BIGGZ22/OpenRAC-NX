/* func_0015BB80 -- SCUS_976.23 0x0015BB80, 128 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 @ 0x0015BB80 - 128 bytes
 *   addiu  $sp, $sp, -0x50 ; sq $s3, 0x30($sp) ; sq $ra, 0x40($sp)
 *   daddu  $s3, $a1, $zero ; sq $s2, 0x20($sp) ; sq $s1, 0x10($sp) ; sq $s0, 0($sp)
 *   beqz   $s3, end         ; addiu $v0, $zero, 0x25
 *   lw     $v0, 0x14($s3) ; sltiu $v0, $v0, 0x3
 *   beqz   $v0, err         ; addiu $a1, $a0, 0x81C
 *   lw     $s0, 0x0($s3) ; lw $s2, 0x1C($s3) ; daddu $a0, $s3, $zero ; lw $s1, 0x3C($s3)
 *   jal    func_184704      ; daddu $a2, $s0, $zero
 *   sw     $s1, 0x3C($s3) ; daddu $v0, $zero, $zero ; sw $s0, 0x0($s3)
 *   b      end              ; sw $s2, 0x1C($s3)
 *   err: addiu $v0, $zero, 0x25
 *   end: lq $ra, 0x40($sp) ... jr $ra ; addiu $sp, $sp, 0x50
 *   Authenticator: a null item gives 37; so does an item whose unsigned
 *   counter at 0x14 is not below three.  Otherwise the three words at 0x00,
 *   0x1C and 0x3C are saved, func_184704 is called with the item, the field of
 *   the owner at 0x81C and the word at 0x00, and the three saved words are
 *   written back.
 *   Proof: the boot harness (chain: SN ProDG ee-gcc 2.95.2 + pinned GNU as and
 *   SN ld, cc1 flags -O2 -G0) compiles this source to the retail bytes of
 *   0x0015BB80, byte for byte.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Boot_0015BB80_Item
{
    int          field_000;
    unsigned char pad_004[0x10];
    unsigned int field_014;
    unsigned char pad_018[0x04];
    int          field_01C;
    unsigned char pad_020[0x1C];
    int          field_03C;
} Boot_0015BB80_Item;

void func_184704(void *item, void *target, int value);

int func_0015BB80(void *self, Boot_0015BB80_Item *item)
{
    int saved_000;
    int saved_01C;
    int saved_03C;

    if (item == 0)
        return 37;
    if (item->field_014 >= 3)
        return 37;
    saved_000 = item->field_000;
    saved_01C = item->field_01C;
    saved_03C = item->field_03C;
    func_184704(item, (void *)((char *)self + 0x81C), saved_000);
    item->field_000 = saved_000;
    item->field_01C = saved_01C;
    item->field_03C = saved_03C;
    return 0;
}
