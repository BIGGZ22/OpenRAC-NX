#include "include_asm.h"

/* Measured partial C views; original types and object ownership are unknown. */
typedef struct {
    unsigned char prefix[0x188];
    unsigned char wrapped;
    unsigned char value;
} QwenRecovery_538934d89776_WrapperCollectState132;

extern unsigned char ref_L00_002D6FD8_01[3];

extern int ref_L00_002D6FD8_00;

extern QwenRecovery_538934d89776_WrapperCollectState132 ref_L00_002D6FD8_03;

extern int ref_L00_002D6FD8_02(int words[2]);

void func_L00_002D6FD8(void)
{
    int words[4];
    ref_L00_002D6FD8_02(words);
    if (ref_L00_002D6FD8_01[2] != 0) {
        ref_L00_002D6FD8_00 += words[0];
        if (ref_L00_002D6FD8_00 >= 256) {
            ref_L00_002D6FD8_03.wrapped = 1;
            ref_L00_002D6FD8_00 -= 255;
        } else {
            ref_L00_002D6FD8_03.wrapped = 0;
        }
        ref_L00_002D6FD8_03.value = ((unsigned char *)words)[4];
    } else {
        words[0] = 0;
        words[1] = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002D7060);

INCLUDE_ASM("asm/levels", func_L00_002D7068);

INCLUDE_ASM("asm/levels", func_L00_002D70A0);

INCLUDE_ASM("asm/levels", func_L00_002D7138);
