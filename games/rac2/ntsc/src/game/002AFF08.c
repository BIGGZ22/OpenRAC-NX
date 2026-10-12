#include "include_asm.h"

/* Qwen draft (branch qwen/autonomy, 9650d156), checked by tools/check.py */
// Helper function declarations, using exact symbol names from externals
void func_002AFDD8(unsigned long, unsigned long, unsigned long, unsigned long);

unsigned int func_00282E20(unsigned char (*)[16]);

// Implementation of func_002AFF08
void func_002AFF08(unsigned long arg1, unsigned long arg2, unsigned long arg3) {
    unsigned char stack_buffer[16];

    func_002AFDD8(arg1, (unsigned long)stack_buffer, arg2, arg3);
    func_00282E20((unsigned char (*)[16])stack_buffer);
}

float func_002AFF38(unsigned long first, unsigned long second, unsigned long third) {
    float result[4];
    func_002AFDD8(first, (unsigned long)result, second, third);
    return result[2];
}

INCLUDE_ASM("asm/game", func_002AFF60);
