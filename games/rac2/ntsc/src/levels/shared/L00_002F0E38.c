#include "include_asm.h"

typedef int s32;

extern s32 D_L24_001BE840;

extern s32 D_L24_001BE888;

typedef unsigned int u32;

extern u32 D_L24_001BE894;

/* returns the signed word at 0x001be840 */
s32 func_L00_002F0E38(void) {
 return D_L24_001BE840;
}

/* returns the signed word at 0x001be888 */
s32 func_L00_002F0E48(void) {
 return D_L24_001BE888;
}

/* stores the provided word at 0x001be894 */
void func_L00_002F0EA8(u32 value) {
 D_L24_001BE894=value;
}

extern short D_L00_001BEF84[];

void func_L00_002F3478(void) {
    D_L00_001BEF84[0] = 0;
}
