/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L08_003CDB38(u8 *p) {
    u8 *s = *(u8 **)(p + 0x68);
    qcopy(p + 0x10, s);
    qcopy(p + 0xF0, s + 0x10);
    func_002A0F48(p);
}
