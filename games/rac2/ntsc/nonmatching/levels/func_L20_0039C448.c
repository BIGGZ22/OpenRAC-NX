/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L20_0039C448(char *arg0) {
    char *s0 = arg0 + 0x10;
    char *s3 = *(char **)(arg0 + 0x68);
    char *s1;
    char buf[0x30];

    qcopy(s3, s0);
    s1 = s3 + 0x10;
    func_00282CC0(s0, s0, s1);
    func_002A8C00(buf, (int)arg0, 0x10001, s1, *(float *)(s3 + 0x2C));
    if (func_002761F8(s3, s0, 0, *(int *)(arg0 + 0xB8), buf) != 0
        || func_00282948(s3 + 0x20) != 0) {
        func_L20_0039C8C8(D_L20_001C0420, 0x14, 1);
        func_002E5DA0(2, 0, arg0, 0x12EF);
        func_0029F338((int)arg0);
    } else {
        func_L20_0039C5D0(arg0);
        func_L00_002E6EF8(func_L20_0039CC10, (int)arg0);
    }
}
