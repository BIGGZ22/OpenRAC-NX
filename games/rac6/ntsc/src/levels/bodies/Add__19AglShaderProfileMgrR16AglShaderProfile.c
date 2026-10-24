/* Add__19AglShaderProfileMgrR16AglShaderProfile -- FE.REL 0x0006ED08, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   addiu $sp, $sp, -0x20
 *   sw    $a0, 0x0($sp)
 *   daddu $a1, $sp, $zero
 *   lui   $a0, 0x22
 *   sd    $ra, 0x10($sp)
 *   jal   Append__t9ApeTArray1ZP16AglShaderProfileRCP16AglShaderProfile
 *   addiu $a0, $a0, 0x5440
 *   ld    $ra, 0x10($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x20
 *   The element appended to the static list is the manager itself; the profile
 *   reference is not read by the body.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef void AglShaderProfile;

extern void Append__t9ApeTArray1ZP16AglShaderProfileRCP16AglShaderProfile(
    void *list, AglShaderProfile *const *elem);
extern void *_19AglShaderProfileMgr$s_profileList;

void Add__19AglShaderProfileMgrR16AglShaderProfile(void *self,
                                                    AglShaderProfile const *profile)
{
    AglShaderProfile *entry = (AglShaderProfile *)self;

    Append__t9ApeTArray1ZP16AglShaderProfileRCP16AglShaderProfile(
        &_19AglShaderProfileMgr$s_profileList, &entry);
}
