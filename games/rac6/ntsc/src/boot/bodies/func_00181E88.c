/* func_00181E88 -- SCUS_976.23 0x00181E88, 128 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 @ 0x00181E88 - 128 bytes
 *   lui $v0,0x18 ; lui $v1,0x29 ; lui $a1,0x21 ; lui $a2,0x20
 *   addiu $sp,$sp,-0x50 ; addiu $v0,$v0,0x1E20 ; addiu $v1,$v1,-0x1C30
 *   addiu $a1,$a1,-0x1F10 ; addiu $a2,$a2,0x44E0 ; addiu $a3,$zero,0x800
 *   sd $s0,0x30($sp) ; daddu $a0,$sp,$zero ; sd $ra,0x40($sp)
 *   sw $v0,0x4($sp) ; sw $v1,0x8($sp) ; sw $a3,0xC($sp) ; sw $a1,0x10($sp)
 *   sw $a2,0x20($sp)
 *   jal func_190B60 ; sw $zero,0x14($sp)
 *   daddu $s0,$v0,$zero
 *   bgez $s0, call ; daddu $a0,$s0,$zero
 *   b end ; addiu $v0,$zero,-1
 *   call: jal func_191EA0 ; daddu $a1,$zero,$zero
 *   daddu $v0,$s0,$zero
 *   end: ld $ra,0x40($sp) ; ld $s0,0x30($sp) ; jr $ra ; addiu $sp,$sp,0x50
 *   Authenticator: a local record is filled with the addresses of four
 *   globals (0x181E20, 0x28E3D0, 0x20E0F0, 0x2044E0), the constant 0x800 and
 *   a zero, then handed to func_190B60; a non negative answer is passed to
 *   func_191EA0 and returned, a negative one becomes -1.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int g_boot_00181E20;
extern int g_boot_0028E3D0;
extern int g_boot_0020E0F0;
extern int g_boot_002044E0;

typedef struct Boot_00181E88_Record
{
    int          pad_000;
    void        *field_004;
    void        *field_008;
    int          field_00C;
    void        *field_010;
    int          field_014;
    int          pad_018[2];
    void        *field_020;
} Boot_00181E88_Record;

int func_190B60(Boot_00181E88_Record *record);
void func_191EA0(void *object, int flag);

int func_00181E88(void)
{
    Boot_00181E88_Record record;
    int result;

    record.field_004 = &g_boot_00181E20;
    record.field_008 = &g_boot_0028E3D0;
    record.field_00C = 0x800;
    record.field_010 = &g_boot_0020E0F0;
    record.field_014 = 0;
    record.field_020 = &g_boot_002044E0;
    result = func_190B60(&record);
    if (result < 0)
        return -1;
    func_191EA0(result, 0);
    return result;
}
