#include "include_asm.h"

typedef unsigned char u8;
typedef short s16;
typedef unsigned int u32;
typedef int s32;

/* func_L12_004254A8 -- 104 bytes, level 12.  Candidate is the exact size and 23 of 26
 * words match; the three left are one register-allocation cluster.
 *
 * retail:
 *   0x004254A8  addiu sp,sp,-0x10 / sd s0,0x0(sp) / sd ra,0x8(sp) / s0 = a0
 *   0x004254B8  lbu v0,0x20(s0) ; bnel v0,zero,epilogue ; ld s0,0x0(sp) [delay]
 *   0x004254C4  lw v0,0x68(s0) ; addiu v1,zero,-1 ; lw a1,0x0(v0) ; beq a1,v1,epilogue
 *   0x004254D8  jal func_002ACBC8 (a0 = 0x00189EA0) ; beqz v0,epilogue
 *   0x004254E4  lui v1,0x14 ; addiu v0,zero,1 ; sb v0,-0x6A25(v1)
 *   0x004254F4  jal func_002A7750 (a0 = s0, a1 = 1, a2 = -1)
 *
 * verdict: wslcmp.py on src/levels/12_todano/L12_0041E3E8.c says
 *   func_L12_004254A8: 0 of 1 places match   [12_todano: instruction at +0x20 differs]
 * measured: this cc1 emits `addiu $a0,$zero,-1` + `lw $v1,0x0($v0)` + `beq $v1,$a0`
 * where retail has the -1 in `$v1` and the field in `$a1`.  Twelve spellings of the
 * guard were measured and all give that same cluster - a plain comparison, a named
 * local, an indexed typed pointer, the constant on the left, a goto-shaped body, and
 * the inverted `if (v != -1) { ... }` form - so it is the register allocator's choice,
 * not the source.  Writing BOTH level addresses as file symbols is what fixed the rest:
 *   `extern u8 D_L12_00189EA0[];` `extern u8 D_L12_001395DB;`
 * (as literals the constant comes out `lui 0x18 / ori 0x9EA0` instead of retail's
 * sign-adjusted `lui 0x19 / addiu -0x6160`, and the store takes `$at`).
 */

extern s32 func_002ACBC8(s32);
extern void func_002A7750(void *, s32, s32);
extern u8 D_L12_00189EA0[];
extern u8 D_L12_001395DB;

void func_L12_004254A8(u8 *a0) {
    if (*(u8 *)(a0 + 0x20) != 0)
        return;
    if (*(s32 *)*(u8 **)(a0 + 0x68) == -1)
        return;
    if (func_002ACBC8((s32)D_L12_00189EA0) == 0)
        return;
    D_L12_001395DB = 1;
    func_002A7750(a0, 1, -1);
}
