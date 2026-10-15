/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point: the shape is right, only the
   quadword copies need a plain-C spelling. Removed from src/ on 2026-10-13, from wave 7. */

void func_L14_003E5030(char *arg0) {
    char *v = *(char **)(arg0 + 0x68);

    qcopy(v + 0x50, arg0 + 0x10);
    qcopy(v + 0x60, arg0 + 0x10);

    if (*(int *)(v + 0x1C) >= 0) {
        *(int *)(v + 0x28) = D_L14_001B259C + (*(int *)(v + 0x1C) << 8);
    }
    if (*(int *)(v + 0x20) >= 0) {
        *(int *)(v + 0x2C) = D_L14_001B259C + (*(int *)(v + 0x20) << 8);
    }
    if (*(int *)(v + 0x24) >= 0) {
        *(int *)(v + 0x30) = D_L14_001B259C + (*(int *)(v + 0x24) << 8);
    }
    if (*(int *)(v + 0x38) >= 0) {
        *(int *)(v + 0x3C) = D_L14_001B259C + (*(int *)(v + 0x38) << 8);
    }
}
