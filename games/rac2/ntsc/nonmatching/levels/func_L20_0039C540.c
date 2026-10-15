/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L20_0039C540(char *arg0, char *arg1, char *arg2, float f12) {
    char *v;
    char *o = func_0029EFD0(0x12F2);

    if (o != 0) {
        *(int *)(o + 0xB8) = (int)arg0;
        qcopy(o + 0x10, arg1);
        v = *(char **)(o + 0x68);
        *(unsigned char *)(o + 0x30) = 0xFF;
        qcopy(v + 0x10, arg2);
        *(float *)(v + 0x2C) = f12;
        *(int *)(v + 0x20) = 0x12C;
    }
    return o;
}
