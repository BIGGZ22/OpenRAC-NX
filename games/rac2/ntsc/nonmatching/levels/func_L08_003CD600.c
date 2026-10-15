/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L08_003CD600(u8 *ctx) {
    u8 *s = D_0018C0B0;
    qcopy(ctx + 0x10, s + 0x10);
    qcopy(ctx + 0xF0, s + 0xF0);
    func_002A0F48(ctx);
}
