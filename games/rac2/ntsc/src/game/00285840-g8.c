#include "include_asm.h"

/* -G8 sibling for src/game/00285840.c: func_002858C0 stores through $gp into the
   small-data word D_001A8A5C, which only comes out gp-relative in a file built
   with -G8. */

typedef int s32;
typedef short s16;

typedef struct {
    char reserved00[0x40];
    s32 field40;
    char reserved44[0x10];
    s32 field54;
    s32 field58;
    s32 field5C[4];
    s32 field6C[4];
    s16 field7C;
    s16 field7E;
    s32 field80;
    char reserved84[4];
    s32 field88;
} Limits_2858C0;

extern Limits_2858C0 D_001BAC80;
extern s32 D_0013955C[];
extern s32 D_001A8BB0;
extern s32 D_001A8A5C __attribute__((sda));

extern s32 func_002B4BE8(void);
extern void func_00132AF8(s32);
extern s32 func_00287E48(void);
extern void func_002B72E8(s32);
extern int func_002E5E58(int, unsigned int, void *);

void func_002858C0(s32 p0, s32 p1)
{
    s32 i;

    D_001A8A5C = p0;
    if (p0 == 5) {
        D_001BAC80.field80 = D_0013955C[0];
    } else {
        D_001BAC80.field80 = 0;
    }
    D_001BAC80.field54 = p1;
    D_001BAC80.field40 = p0;
    D_001BAC80.field7C = 0;
    D_001BAC80.field7E = 0;
    D_001BAC80.field58 = 0;
    D_001BAC80.field88 = (func_002B4BE8() != 3);
    if (D_001A8BB0 != 3) {
        func_00132AF8(0x5D);
        if (func_00287E48() == 0) {
            func_002B72E8(0);
        }
    }
    for (i = 0; i < 4; i++) {
        D_001BAC80.field5C[i] = 0;
        D_001BAC80.field6C[i] = 0;
    }
    if (p0 == 10) {
        func_002E5E58(0x12, 0, 0);
    }
}
