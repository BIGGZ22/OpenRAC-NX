#include "include_asm.h"

extern int F46b43b72_D_0014B540[];

extern int D_001A7288;

extern s32 func_002B7C10(s32 a0, s32 a1, s32 a2);

/* Near miss for func_002939D0 (boot 0x002939D0, 27 levels, 128 bytes).
 *
 * The whole body is recovered: the 0x1057-granular arena address
 *
 *   (0x1FF7FF0 - ((field_0x34C << 11) + 0x1057) & 0xFFFFF000) & 0xFFFFFFF0
 *
 * is stored in D_001A7288, 0x60 is written through it and func_002B7C10 is called with
 * D_001A7288 + *D_001A7288 and two table fields.  Three words are left: retail keeps the
 * final `& 0xFFFFFFF0` (this cc1 folds it away, the value only ever reaches an
 * address), retail materialises 0x1FF7FF0 as `lui`+`addiu` and this cc1 as
 * `lui`+`ori`, and retail re-materialises D_001A7288's address for the reload where
 * this cc1 keeps it in a register.
 */
int func_002939D0(void) {
    int *b = F46b43b72_D_0014B540;
    unsigned int v = b[211];
    unsigned int addr = (0x1FF7FF0 - (((v << 11) + 0x1057) & 0xFFFFF000u)) & 0xFFFFFFF0u;
    D_001A7288 = (int)addr;
    *(unsigned int *)addr = 0x60;
    func_002B7C10(D_001A7288 + *(int *)D_001A7288, b[210] + b[203], b[211]);
    return 1;
}
