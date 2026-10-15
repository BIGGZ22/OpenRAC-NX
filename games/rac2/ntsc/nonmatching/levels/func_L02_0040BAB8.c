/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L02_0040BAB8(u8 *a0, float *a1) {
    float v, t, h, k;
    u8 *q;

    v = func_00283248(*(float *)(a0 + 0x10) - a1[0], *(float *)(a0 + 0x14) - a1[1]);
    v = v + func_002A7878(-0.7853982448577881f, 0.7853982448577881f);
    t = func_002A7878(-2.0f, 2.0f) + 8.0f;
    q = *(u8 **)(a0 + 0x68);
    qcopy(q + 0x2C0, a1);
    h = t * func_00283180(v);
    *(float *)(q + 0x2C0) = *(float *)(q + 0x2C0) + h;
    k = func_00283198(v);
    *(float *)(q + 0x2C4) = *(float *)(q + 0x2C4) + t * k;
}
