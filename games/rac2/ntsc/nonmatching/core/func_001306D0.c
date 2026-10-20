#include "common.h"

#define GS_ADDR_MASK 0x0FFFFFFF
#define GS_ADDR_BANK 0x20000000

/* Nine texture-page addresses: the three given ones, the same three again,
   and the three shifted by one mip level's stride, all in the GS's own
   address form. */
void func_001306D0(u32 *out0, u32 *out1, u32 *out2, u32 *out3, u32 *out4, u32 *out5,
                   u32 *out6, u32 *out7, u32 *out8,
                   u32 first, u32 second, u32 third, s32 width, s32 height) {
    u32 step = width * height / 512 * 0x180;
    u32 a = first & GS_ADDR_MASK | GS_ADDR_BANK;
    u32 b = second & GS_ADDR_MASK | GS_ADDR_BANK;
    u32 c = third & GS_ADDR_MASK | GS_ADDR_BANK;
    u32 a2 = (step + first) & GS_ADDR_MASK | GS_ADDR_BANK;
    u32 b2 = (step + second) & GS_ADDR_MASK | GS_ADDR_BANK;
    u32 c2 = (step + third) & GS_ADDR_MASK | GS_ADDR_BANK;

    *out0 = a;
    *out1 = b;
    *out2 = c;
    *out3 = a;
    *out4 = b;
    *out5 = c;
    *out6 = a2;
    *out7 = b2;
    *out8 = c2;
}
/* Near miss: 216 bytes against retail's 224, same frame (0x50) and same
   callee-saved set.
   Six masked GS addresses and the nine stores are right. The 8-byte gap is the
   stride: retail materialises width*height/512*0x180 three times (three
   `mult` into $v1, $v1, $a0 at +0x48/+0x70/+0x74) and reloads the 0x0FFFFFFF
   and 0x20000000 masks from the stack for the tail, where this candidate keeps
   the stride in one register and the masks in $t-registers. Note for whoever
   reads the listing: those `mult $rs,$rt` words are the R5900's three-operand
   `mult rd,rs,rt`, which rabbitizer does not decode. */
