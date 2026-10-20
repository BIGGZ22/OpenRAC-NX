/* 0 of 1 places match - 128-bit LQ/SQ copy; MEASURED not reachable from C.
   Level 3 (03_endako), 0x0041F048, 72 bytes:
     0031B920(a0, 7, 2) after the full frame save; then
       lw    $s0,0x68($v0)
       addiu $s0,$s0,0x1B0
       lq    $v0,0x0($s1)      (0x7A220000, opcode 0x1E = LQ)
       sq    $v0,0x0($s0)      (0x7E020000, opcode 0x1F = SQ)
       ... epilogue
   i.e. a 16-byte vector copy: *(V4 *)(ctx->field_68 + 0x1B0) = *src.
   Four spellings were compiled this session (Vector4 aggregate assignment, four
   scalar f32 stores, the copy before/after the call, char* + cast): all give 92-96
   bytes of ld/sd word moves and this cc1 never emits lq/sq, so the retail's two
   128-bit instructions cannot come out of C here. Needs ASM_FUNC (lead decision
   for the EE 128-bit moves). Same result as func_L02_003A8770 / func_L07_002E2090. */

#include "include_asm.h"

typedef float Vector4[4];

extern void func_0031B920(void *, int, int);

void func_L03_0041F048(char *self, Vector4 *src)
{
    Vector4 *dst = (Vector4 *)(*(char **)(self + 0x68) + 0x1B0);

    func_0031B920(self, 7, 2);
    *dst = *src;
}
