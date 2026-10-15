/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L14_00390DF8(char *arg0, char *arg1, char *arg2) {
    char buf[16];

    qcopy(buf, D_0018A0E0);
    ((void (*)(void *, void *, float))func_2F5CC0)(buf, buf, -1.0f);
    qcopy(arg0 + 0x30, arg1);
    qcopy(arg0, arg2);
    {
        float one = 1.0f;

        func_2F5F18(arg0, arg0, one);
        func_2F5D68(arg0 + 0x10, arg0, buf);
        func_2F5F18(arg0 + 0x10, arg0 + 0x10, one);
        func_2F5D68(arg0 + 0x20, arg0 + 0x10, arg0);
    }
}
