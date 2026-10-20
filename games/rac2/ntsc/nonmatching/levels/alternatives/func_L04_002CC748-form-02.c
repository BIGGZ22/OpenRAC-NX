/* Form 2: the guard reads one C name for D_00189E20 and the tail a second name aliased onto
   the same address (`__asm__`), which stops the address CSE. This is the only measured shape
   that reaches retail's EXACT 144 bytes; the two `lui`/`addiu` pairs it emits are one
   instruction apart from retail's shared-`$hi` pair, so the block offsets still differ.
   Naming/extraction only: this form has not been recompiled or validated.
   See levels/func_L04_002CC748.c for the primary entry, the retail reading and the wall. */

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef float f32;

typedef struct {
    u8 pad00[0x7A8];
    f32 field7A8;
    f32 field7AC;
    f32 field7B0;
    f32 field7B4;
    u8 pad7B8[0x1492 - 0x7B8];
    u8 b1492;
    u8 pad1493[0x2290 - 0x1493];
    u32 field2290;
    u32 field2294;
    u8 pad2298[0x22BA - 0x2298];
    u8 b22BA;
    u8 pad22BB[0x24CC - 0x22BB];
    s32 w24CC;
    u8 pad24D0[0x24EC - 0x24D0];
    s16 h24EC;
} D00189E20F;

extern D00189E20F D_00189E20F;
extern D00189E20F D_00189E20G __asm__("D_00189E20F");

extern char D_00139568[];
extern char D_L04_0027B470[];
extern int func_L00_002ADEA0(int);

int func_L04_002CC748(int a0) {
    if (a0 != 0 && D_00189E20F.b22BA != 0 && D_00189E20F.h24EC == 0)
        return *(int *)((int)D_L04_0027B470 +
                        *(u8 *)(func_L00_002ADEA0(0) + (int)D_00139568) * 0xE0 + 0x28);
    if (D_00189E20G.w24CC == 1)
        return 0x51;
    if (D_00189E20G.b1492 == 0)
        return 0;
    return 0x67;
}
