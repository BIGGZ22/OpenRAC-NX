/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L20_0039F210(char *arg0, char *arg1, int arg2) {
    char *state;

    qcopy(arg0 + 0x10, arg1);
    state = *(char **)(arg0 + 0x68);
    *(int *)(state + 0x0) = 5;
    *(int *)(state + 0xC) = arg2;
}
