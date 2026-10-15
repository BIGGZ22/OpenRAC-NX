/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L20_0039E158(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *v;

    if (*(int *)(state + 0x27C) != 0) {
        *(int *)(state + 0x280) = 0;
        func_002B3BA8(arg0, *(int *)(state + 0x230), 0.0f, 0.0f, 0, 0);
    } else {
        *(char **)(state + 0x280) = func_L20_004448A8(*(char **)(arg0 + 0xB8), arg0);
        if (*(char **)(state + 0x280) == 0) {
            *(char **)(state + 0x280) = *(char **)(arg0 + 0xB8);
        }
        v = *(char **)(state + 0x280);
        *(int *)(state + 0x200) = (int)v;
        qcopy(state + 0x1F0, v + 0x10);
    }
}
