/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L20_0043D470(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(float *)(*(char **)(state + 0x84) + 0x0) = 8.0f;
    *(float *)(*(char **)(state + 0x84) + 0x4) = 2.5f;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x18) = 1;
    *(unsigned short *)(*(char **)(state + 0x84) + 0x1A) = 0;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x1E) = 1;
    *(unsigned char *)(*(char **)(state + 0x84) + 0x1F) = 0;
    qcopy(*(char **)(state + 0x84) + 0x20, state + 0x60);
}
