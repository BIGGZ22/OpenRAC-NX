/* FastMemCopy -- 16-byte-granular copy helper (RAC5 0x01E68EE0, 32 bytes).
 * Date: 2026-10-17 (vague 12, lot w12/q28).
 * Provenance: no C body exists in the corpora -- RAC1 lombyte unit
 *   src/assembly/textbin/fun_001f9a98.c is intentionally assembly
 *   (INCLUDE_ASM, "intentional low-level assembly"), and R1PAL
 *   src/game/fastfunc.c declares ASM_FUNC("asm/handwritten/text",
 *   func_001F9A98). Body reconstructed from the retail disassembly:
 *   memcpy(dst, src, n << 4); the shift fills the call delay slot.
 */
extern void *memcpy(void *dst, const void *src, unsigned int n);

void FastMemCopy__FPvPCvUi(void *dst, const void *src, unsigned int n)
{
    memcpy(dst, src, n << 4);
}
