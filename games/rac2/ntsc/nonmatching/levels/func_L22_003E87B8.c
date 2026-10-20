/* 0 of 1 places match - last line: 22_dobbo_orbit: instruction at +0x2c differs
   Right size (120 bytes). Spawns into the caller's object, paints a font-mode string and
   arms the object's animation, then hands it to the engine.  Two differences: this cc1
   emits the `addiu $a0,$v0,0xC` of the fourth argument BEFORE the `addiu $a3,$zero,0xE0B`
   constant where retail has the constant first, and the tool prints the four `jal`s
   without their relocation (those are fine).  Tried and identical: the constant parked in
   a named local, the font pointer parked in a local.  Needs the argument-setup order of
   the `func_362DE0` call (a3 materialised before the call's a0). */

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef float f32;

extern s32 func_321F48(s32, s32, s32);
extern u8 *func_31FC88(s32);
extern void func_362DE0(u8 *, s32, s32, s32);
extern void func_325308(s32, s32, s32, f32, f32);
extern void func_3159F0(s32);

void func_L22_003E87B8(s32 a0, s32 a1, s32 a2) {
    if (func_321F48(a0, -1, 0) == 0)
        return;
    func_362DE0(func_31FC88(3) + 0xC, 0, a0, 0xE0B);
    func_325308(a0, a0 + 0x10, -1, 2.0f, 13.0f);
    func_3159F0(a0);
}
