#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef int s32;

/* the level's object-handle table entry: 0x100 bytes, indexed by the handle */
typedef struct {
    u8 pad00[0x68];
    void *f68;
    u8 pad6C[0x100 - 0x6C];
} Ent;

/* what an entry points at; only the queue flag and its sound handle are named here */
typedef struct {
    u8 pad00[0x18C];
    u8 f18C;
    u8 pad18D[0x1CC - 0x18D];
    s32 f1CC;
} Moby;

extern s32 func_2F9A60();

INCLUDE_ASM("asm/levels", func_L08_003D8348);

INCLUDE_ASM("asm/levels", func_L08_003D8A28);

INCLUDE_ASM("asm/levels", func_L08_003D8AB0);

INCLUDE_ASM("asm/levels", func_L08_003D8CA8);

INCLUDE_ASM("asm/levels", func_L08_003D9038);

INCLUDE_ASM("asm/levels", func_L08_003D9208);

INCLUDE_ASM("asm/levels", func_L08_003D92D8);

INCLUDE_ASM("asm/levels", func_L08_003D9798);

INCLUDE_ASM("asm/levels", func_L08_003D9F68);

INCLUDE_ASM("asm/levels", func_L08_003F6D30);

INCLUDE_ASM("asm/levels", func_L08_003F6E18);

INCLUDE_ASM("asm/levels", func_L08_003F76C0);

INCLUDE_ASM("asm/levels", func_L08_003F77C0);

INCLUDE_ASM("asm/levels", func_L08_003F79E0);

INCLUDE_ASM("asm/levels", func_L08_003F7A48);

INCLUDE_ASM("asm/levels", func_L08_0040E9B8);

INCLUDE_ASM("asm/levels", func_L08_0040F9D0);

INCLUDE_ASM("asm/levels", func_L08_0040FAE8);

INCLUDE_ASM("asm/levels", func_L08_0041D808);

INCLUDE_ASM("asm/levels", func_L08_00422190);

INCLUDE_ASM("asm/levels", func_L08_00422428);

INCLUDE_ASM("asm/levels", func_L08_00422578);

INCLUDE_ASM("asm/levels", func_L08_00422850);

INCLUDE_ASM("asm/levels", func_L08_00422900);

INCLUDE_ASM("asm/levels", func_L08_00422AD8);

/* the queued sound of the entry is released and the flag cleared */
void func_L08_00422B60(Ent *a0)
{
    Moby *s0 = a0->f68;

    if (s0->f18C != 0) {
        func_2F9A60(s0->f1CC);
        s0->f18C = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L08_00422B98);

INCLUDE_ASM("asm/levels", func_L08_00422EE0);

INCLUDE_ASM("asm/levels", func_L08_00423010);

INCLUDE_ASM("asm/levels", func_L08_004233E0);

INCLUDE_ASM("asm/levels", func_L08_004237F0);

INCLUDE_ASM("asm/levels", func_L08_004238A0);
