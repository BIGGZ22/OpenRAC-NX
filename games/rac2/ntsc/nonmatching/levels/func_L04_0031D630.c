/* func_L04_0031D630, 260 bytes, 8 placements.  ABANDON - two words off.
   The walk of the level's live objects is recovered: the count word at +0x44 of
   the level record, the table at +0x18C, the kind word at +0xAA of each entry
   (0xE29 raises the flag word at +0x34, 0xD54 calls func_002E64A0, 0xFA0 hands
   the owner at +0x68 to func_L00_00326048), the two branch-likely continues
   and the float argument 0x061086535f (retail's 0x3D7A35E0) are all exact.
   The residue is the base address: retail keeps it in a temp ($v0, the
   `lui $v0,0x1C` / `addiu $v0,$v0,-0x2C80` pair) and copies it into $s3 in the
   `blez` delay slot (`daddu $s3,$v0,$zero`), while this cc1 puts the address
   straight into $s3 and reads the count from it.  That also moves $s0 in the
   prologue's save order (retail $s1,$s0,$s2,$s3; here $s1,$s2,$s3,$s0).
   Spellings measured: the address in a temp with the guard reading it and a
   second variable for the walk (the guard then needs its own copy, 268 bytes),
   the same address written twice (the CSE folds it to +0x44/-0x44, 276 bytes),
   and the guard and the walk sharing one variable (260 bytes, the deposit).
   Nine compiles. */

#include "include_asm.h"

typedef signed int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef float f32;

extern void func_L00_002FC980(void);
extern void func_002E64A0(void *object);
extern void func_L00_00326048(void *a0, void *a1, u32 a2, u32 a3, f32 f12);
extern char D_L04_001BF380[];

void func_L04_0031D630(void)
{
    char *s3;
    char *s2;
    char *s0;
    short v;
    int s1;
    int a1;

    func_L00_002FC980();
    s3 = D_L04_001BF380;
    s2 = s3 + 0x18C;
    for (s1 = 0; s1 < *(short *)(s3 + 0x44); s1++, s2 += 4) {
        s0 = *(char **)s2;
        v = *(short *)(s0 + 0xAA);
        if (s0 != 0 && v == 0xE29)
            *(unsigned short *)(s0 + 0x34) |= 0x800;
        if (*(short *)(s0 + 0xAA) == 0xD54)
            func_002E64A0(s0);
        if (s0 != 0 && *(short *)(s0 + 0xAA) == 0xFA0) {
            a1 = *(int *)(s0 + 0x68);
            if (a1 != 0)
                func_L00_00326048(s0, a1 + 0xC, 0x80AAAAD2, 0x80396464,
                                  0.061086535f);
        }
    }
}
