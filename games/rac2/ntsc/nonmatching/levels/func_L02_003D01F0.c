/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L02_003D01F0(char *arg0, char *arg1) {
    char *self = arg0;
    char *p = *(char **)(self + 0x68);
    func_002A7750(self, 4, -1);
    qcopy(p + 0xB0, arg1);
}
