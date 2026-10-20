/* func_L22_003FA1D0, 212 bytes, 1 placement.  Nearest: exact size, ONE word at +0x34.
   Arms the level-22 object: func_321F48(a0, -1, 0) must return non-zero, then
   func_362DE0(func_31FC88(3) + 0xC, 0, a0, 0xE0B), func_325308(a0, a0 + 0x10, -1, 2.0f, 13.0f),
   func_362D38(0, 0, a0), func_2F3D60(buf, s1 + 0x10, 0.25f),
   func_32AED0(a0, 3, buf, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f) and func_3159F0(a0), with the 16-byte
   buffer at sp+0.
   The single gap is the setup order of func_362DE0's arguments: retail materialises the
   constant fourth argument first (`addiu $a3,$zero,0xE0B`) and then the dependent first one
   (`addiu $a0,$v0,0xC`), this cc1 emits them the other way round.  Everything else, registers
   included, is retail's, and the file is the same one the level-22 wave-6 note records for
   the same class (func_L22_003E87B8).  Two more spellings (the inner call in a named pointer,
   the fourth argument named at the top of the body) give the same single word. */

#include "include_asm.h"
#include "common.h"

extern char *func_321F48(char *a0, s32 a1, s32 a2);

extern char *func_31FC88(s32 a0);

extern void func_362DE0(char *a0, s32 a1, char *a2, s32 a3);

extern void func_325308(char *a0, char *a1, s32 a2, f32 f12, f32 f13);

extern void func_362D38(s32 a0, s32 a1, char *a2);

extern void func_2F3D60(void *a0, void *a1, f32 f12);

extern void func_32AED0(char *a0, s32 a1, void *a2, s32 a3, f32 f12, f32 f13, f32 f14,
                        f32 f15);

extern void func_3159F0(char *a0);

void func_L22_003FA1D0(char *a0)
{
    char *s1 = func_321F48(a0, -1, 0);
    char buf[0x10];

    if (s1 == 0)
        return;
    func_362DE0(func_31FC88(3) + 0xC, 0, a0, 0xE0B);
    func_325308(a0, a0 + 0x10, -1, 2.0f, 13.0f);
    func_362D38(0, 0, a0);
    func_2F3D60(buf, s1 + 0x10, 0.25f);
    func_32AED0(a0, 3, buf, 0xFF, 3.0f, 6.0f, 3.0f, 6.0f);
    func_3159F0(a0);
}
