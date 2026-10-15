/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L14_00328E70(char *arg0, char *arg1, char *arg2) {
    qcopy(arg1, arg0 + 0x10);
    qcopy(arg2, arg0 + 0xF0);
}
