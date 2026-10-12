/* 28 bytes against retail's 36: retail is a beql on the null test with the 0 result in its delay slot, then a bnez with addiu 1 / daddu 0 tail; the compiler gives beqz or bnel plus sltu, so the branch-likely shape is missing. */
typedef int s32;
typedef unsigned char u8;

s32 func_L00_0031BE88(u8 *a0)
{
    s32 *p = *(s32 **)(a0 + 0x98);
    s32 r = 0;

    if (p != 0) {
        if (p[2] != 0) {
            r = 1;
        }
    }
    return r;
}
