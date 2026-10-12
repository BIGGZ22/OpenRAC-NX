/* func_L12_0041BDF8 (104 B) and func_L12_0041BE60 (120 B) — best attempts, NOT matching.
 *
 * Both are 8 bytes short: every instruction is in the right place except that the
 * retail keeps the incoming pointer in $a1 as a separate copy of $a0 made at entry
 *   daddu $a1,$a0,$zero
 * while gcc folds the local onto $a0 (mine starts `beqz $a0` / uses $a0 throughout).
 * BE60 additionally wants an explicit `daddu $a0,$a1,$zero` before the func_41BED8 call.
 * Tried: parameter typed u8* and void*, local pointer assigned from it, call passing the
 * local, call passing the parameter, computing p before the call.  Nothing produced the
 * $a1 copy -- the two pseudos coalesce in every spelling tried.
 */

extern void func_L12_0041BED8(u8 *);

void func_L12_0041BDF8(void *self)
{
    u8 *a1 = self;
    s32 i;
    u8 **p;

    if (a1 == 0)
        return;
    if (*(s16 *)(a1 + 0xAA) != 0xFAE)
        return;
    p = (u8 **)(*(u8 **)(a1 + 0x68) + 8);
    func_L12_0041BED8(a1);
    for (i = 1; i >= 0; i--, p++) {
        if (*p != 0)
            *(u32 *)(*p + 0x98) = *(u32 *)(*(u8 **)(*p + 0x24) + 0x10);
    }
}
