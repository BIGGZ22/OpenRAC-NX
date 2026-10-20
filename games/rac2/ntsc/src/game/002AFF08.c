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

/* D_001A8CB0 is `sda`: cc1 emits its address as one `la` insn, which changes the schedule so
   that parameter a is copied to $v0 on entry and moved to $a2 in the call's delay slot, as in
   retail. Without `sda` the copy is coalesced and the function is 116 bytes. */
extern void func_00282CF0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern void func_00282D30(void *, void *, float);
extern char D_001A8CB0[] __attribute__((sda));

void func_002AFF60(char *a, char *b, int c)
{
    func_00282CF0(b, D_001A8CB0, a);
    *(int *)(b + 8) = 0;
    func_00282F20(b, b, 1.0f);
    if (c == 0)
        func_00282D30(b, b, -1.0f);
}
