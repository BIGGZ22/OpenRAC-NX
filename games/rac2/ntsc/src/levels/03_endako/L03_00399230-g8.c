#include "include_asm.h"

#include "common.h"

extern f32  D_L03_001B0340 __attribute__((sda));
extern void func_002A7750(char *a0, s32 a1, s32 a2);
extern s32  func_31BA30(s32 a0, s32 a1);
extern s32  func_31B9F0(s32 a0);
extern f32  func_2F0000(f32 a0, f32 a1);

void func_L03_003CA938(char *a0)
{
    if (*(u8 *)(a0 + 0x31) == 0) {
        return;
    }
    switch (*(u8 *)(a0 + 0x20)) {
    case 0:
        *(u8 *)(a0 + 0xBC) = (u8)func_31BA30(5, 10);
        if (func_31B9F0(2) != 0) {
            *(u8 *)(a0 + 0xBC) += 15;
        }
        func_002A7750(a0, 1, -1);
        break;
    case 1:
        *(f32 *)(a0 + 0xF8) = func_2F0000(*(f32 *)(a0 + 0xF8),
            D_L03_001B0340 * 0.000290888245f * (f32)(*(u8 *)(a0 + 0xBC) - 15));
        break;
    }
}

typedef int TI128 __attribute__((mode(TI)));

extern void func_L03_003CAD78(char *a0);
extern void func_31B920(char *a0, s32 a1, s32 a2);
extern void func_2E77F0(void *a0);
extern char *func_3113F0(s32 a0);
extern void func_32AA20(char *a0, char *a1);
extern f32  func_2EF670(f32 a0, f32 a1);
extern f32  func_2EF110(f32 *a0);

extern f32 D_L03_001B0350[] __attribute__((sda));
extern f32 D_L03_001B0648 __attribute__((sda));

/* case 0 lights the actor's motion, case 1 winds the three tracks down */
INCLUDE_ASM("asm/levels", func_L03_003CACA8);

/* spawns the actor and seeds its aim from the caller's vector */
INCLUDE_ASM("asm/levels", func_L03_003D18F8);

/* The object the level's state points at: the retail writes six floats, a run
   of bytes and shorts, and two counters into it. */
typedef struct {
    f32 f00;
    f32 f04;
    f32 f08;
    f32 f0C;
    f32 f10;
    f32 f14;
    u8  b18;
    u8  b19;
    u16 h1A;
    u16 pad1C;
    u8  b1E;
    u8  b1F;
    u8  pad20[0x10];
    u16 h30;
    u16 h32;
    s32 i34;
} L03_003D21F8_Obj;

/* The object pointer is small data (the retail reaches it through $gp in
   ordinary flow, not only in a delay slot), the state word is not: the retail
   reads it with an absolute lui/%lo pair and clears it from a delay slot. */
extern L03_003D21F8_Obj *D_L03_001B0670 __attribute__((sda));
extern u32 D_L03_001B3458;
extern void *func_L00_002D70A0(s32 size);

/* case 0 spawns the object and points the actor at it; case 1 clears the
   object while the state word still reads zero, and clears the word otherwise */
void func_L03_003D21F8(char *a0)
{
    char *s0 = a0;

    switch (*(u8 *)(s0 + 0x20)) {
    case 0:
        D_L03_001B0670 = func_L00_002D70A0(4);
        func_002A7750(s0, 1, -1);
        *(u8 *)(s0 + 0x30) = 0xFF;
        break;
    case 1: {
        u32 v = D_L03_001B3458;

        if (v == 0) {
            D_L03_001B0670->f00 = -1.0f;
            D_L03_001B0670->f04 = -1.0f;
            D_L03_001B0670->f08 = -1.0f;
            D_L03_001B0670->f0C = -1.0f;
            D_L03_001B0670->f10 = -1.0f;
            D_L03_001B0670->f14 = -1.0f;
            D_L03_001B0670->b18 = 0;
            D_L03_001B0670->b19 = 0;
            D_L03_001B0670->h1A = 0;
            D_L03_001B0670->b1E = 0;
            D_L03_001B0670->b1F = 0;
            D_L03_001B0670->h30 = 0;
            D_L03_001B0670->i34 = 6;
            D_L03_001B0670->h32 = 120;
        } else {
            D_L03_001B3458 = 0;
        }
        break;
    }
    }
}
