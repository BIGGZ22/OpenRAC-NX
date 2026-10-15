#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed char s8;
typedef signed short s16;
typedef unsigned int u32;
typedef int s32;
typedef float f32;

/* Level data: the first is read $gp-relative, the second absolutely (an incomplete
   array type keeps it out of the small-data area even under -G8). */
extern s32 D_L22_001AF314 __attribute__((sda));
extern u8 D_L22_0018B694[];

s32 func_L22_003A6620(void) {
    s32 a0 = D_L22_001AF314;
    s32 a1 = *(s32 *)D_L22_0018B694;

    return a1 < *(s16 *)(*(u32 *)(a0 + 0x68) + 0x324);
}
