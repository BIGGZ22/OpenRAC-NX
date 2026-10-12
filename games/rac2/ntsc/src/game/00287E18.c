#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

typedef struct {
    u8 prefix[0x40];
    s32 mode;
    u8 between[0x14];
    s32 state;
} StateTransitionView;

extern StateTransitionView D_L00_001BEF00;

/* Owner-reviewed Qwen backlog: observed operations, not recovered original object boundaries. */

/* func_00285840: owner ABI/declaration review retained. */
/* extern declaration for global variable referenced in func_00285840 */
extern unsigned int D_001BACC0;

/* Change resident state word from1 to2 only when the mode word equals7 */
void func_00287E18(void) {
    if (D_L00_001BEF00.mode == 7 && D_L00_001BEF00.state == 1) {
        D_L00_001BEF00.state = 2;
    }
}

int func_00287E48(void)
{
    return D_001BACC0 == 7;
}
