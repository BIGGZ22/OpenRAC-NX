/* from src/core/0012F940.c: 76 bytes against retail's 80 */
/*
 * Fills a descriptor through the pointer at +0x40: the 1 at +0xB0, the masked
 * and tagged second argument at +0xD8, the third shifted left by four at
 * +0xE4, two zeros at +0xDC/+0xE0, then func_0012FBF0 with the first argument.
 *
 * Retail opens with `sll $a2,$a2,4 ; nop` *before* the prologue - the shift
 * sits ahead of the frame, with a nop after it - and orders the stores
 * 0xB0, 0xD8, 0xE4, 0xDC with 0xE0 in the call's delay slot; this cc1
 * schedules the shift into the middle of the body, orders the stores
 * 0xD8, 0xB0, 0xE4, 0xE0 and puts 0xDC in the delay slot, which is exactly
 * one instruction (the nop) short. Two spellings tried (direct expression,
 * then the mask in a local); both give the same 76 bytes.
 */
extern void func_0012FBF0(int arg0);

void func_0012F948(int arg0, unsigned int arg1, int arg2) {
    int *p = *(int **)((char *)arg0 + 0x40);

    arg2 <<= 4;
    p[0xB0 / 4] = 1;
    p[0xD8 / 4] = (arg1 & 0x0FFFFFFF) | 0x20000000;
    p[0xE4 / 4] = arg2;
    p[0xDC / 4] = 0;
    p[0xE0 / 4] = 0;
    func_0012FBF0(arg0);
}
