#include "include_asm.h"

typedef struct { int v[12]; } Blob;

extern Blob D_L00_001A8E60;

/* Find a mapped record by its unsigned halfword class and return its opaque word. */
typedef struct {
    unsigned int marker;
    unsigned char reserved04[0x38];
    unsigned short class_code;
    unsigned char reserved3E[0xa2];
} MappedClassEntry;

extern unsigned char D_00139568[];

extern MappedClassEntry D_L00_00262570[];

typedef int s32;

typedef unsigned char u8;

/* func_002889B8 : clears three fields, initializes a block, then sets two flags */
extern s32 func_00115484(u8 *, s32, s32);

extern void func_002889B8(unsigned char *);

typedef unsigned int u32;

/* 48-byte `ldl/ldr` → `sdl/sdr` stack copy of an absolute global, then a */
int func_002880E0(int x)
{
    Blob b;
    int i;
    b = D_L00_001A8E60;
    for (i = 0; b.v[i]; i++)
        if (x == b.v[i])
            return 1;
    return 0;
}

INCLUDE_ASM("asm/game", func_00288188);

INCLUDE_ASM("asm/game", func_00288208);

INCLUDE_ASM("asm/game", func_00288308);

INCLUDE_ASM("asm/game", func_002883E8);

/* search56 byte-mapped E0 records by U16class3C, return opaque32 marker0 elsezero */
unsigned int func_002884B0(unsigned int key)
{
    int i;
    for (i = 0; i < 56; i++) {
        MappedClassEntry *record = &D_L00_00262570[D_00139568[i]];
        if (record->class_code == key)
            return record->marker;
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_00288508);

INCLUDE_ASM("asm/game", func_002886C8);

INCLUDE_ASM("asm/game", func_00288768);

INCLUDE_ASM("asm/game", func_002887B0);

INCLUDE_ASM("asm/game", func_00288870);

INCLUDE_ASM("asm/game", func_00288970);

INCLUDE_ASM("asm/game", func_002889B0);

void func_002889B8(u8 *a0) {
    *(s32 *)(a0 + 0x30) = 0;
    *(s32 *)(a0 + 0x34) = 0;
    *(s32 *)(a0 + 0x38) = 0;
    func_00115484(a0 + 8, 0xcd, 0x28);
    *(s32 *)(a0 + 0x44) = 0;
    *(s32 *)(a0 + 0x40) = 1;
}

s32 func_00288A00(u8 *a0,s32 *a1,s32 *a2) {
 s32 *values,*flags; s32 idx;
 if(*(s32 *)(a0+0x38)==0) return 0;
 if(--*(s32 *)(a0+0x38)==0) *(s32 *)(a0+0x44)=0;
 values=(s32 *)(a0+8); flags=(s32 *)(a0+12);
 *a1=*(s32 *)((u32)values+(*(s32 *)(a0+0x34)<<3));
 *a2=*(s32 *)((u32)flags+(*(s32 *)(a0+0x34)<<3));
 values=(s32 *)((u32)values+(*(s32 *)(a0+0x34)<<3)); *values=-1;
 flags=(s32 *)((u32)flags+(*(s32 *)(a0+0x34)<<3)); *flags=-1;
 idx=*(s32 *)(a0+0x34)+1;
 if(idx==5) idx=0;
 *(s32 *)(a0+0x34)=idx;
 return 1;
}

s32 func_00288AA0(u8 *a0,s32 a1) {
 s32 v;
 if(*(s32 *)(a0+0x38)==5 || *(s32 *)(a0+0x44)!=0) return 0;
 *(s32 *)((u32)a0+(*(s32 *)(a0+0x30)<<3)+8)=a1;
 *(u32 *)((u32)a0+(*(s32 *)(a0+0x30)<<3)+12)=(u32)(a1-0x20U)<0x91;
 v=*(s32 *)(a0+0x30)+1;
 if(v==5) v=0;
 *(s32 *)(a0+0x38)=*(s32 *)(a0+0x38)+1;
 *(s32 *)(a0+0x30)=v;
 return 1;
}
