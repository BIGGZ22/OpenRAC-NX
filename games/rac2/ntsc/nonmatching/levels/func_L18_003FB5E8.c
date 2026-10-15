/* Not landable: this body reaches a 128-bit copy through `qcopy`/`qzero` or a local
   `__asm__` helper, and AGENTS.md's "Plain C" rule (no inline assembly in a function)
   forbids that as a matching route. Kept as a starting point. Removed from src/ 2026-10-13. */

func_L18_003FB5E8(BatchA_A18_u8 *a0, BatchA_A18_u8 *a1) {
    BatchA_A18_u8 *p = *(BatchA_A18_u8 **)(a0 + 0x68);
    qcopy(a1, p);
    *(float *)(a1 + 8) = *(float *)(a1 + 8) - 1.0f;
}
