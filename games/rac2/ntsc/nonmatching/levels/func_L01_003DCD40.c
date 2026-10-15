/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L01_003DCD40(u8 *a0, void *a1) {
    u8 *o = (u8 *)*(s32 **)(a0 + 0x68);
    s32 list[0x10];
    u8 vec[0x10];
    s32 count;

    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC8B8(a0, list, &count);
    }
    qcopy(vec, a0 + 0x10);
    func_L01_00318638(a0, a1, o + 0x1C0);
    func_L01_003DC9E0(a0, vec);
    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC9A8((void **)list, &count);
    }
}
