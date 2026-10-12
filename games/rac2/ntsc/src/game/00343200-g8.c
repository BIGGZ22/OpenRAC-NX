#include "include_asm.h"
typedef unsigned char u8;

extern int func_00343060(u8 *);
extern void func_00335E68(u8 *, int);
extern void func_00335E38(void *, float, float, float, float);
extern float D_001AE214;
extern float D_001AE210 __attribute__((sda));
void func_003435C8(u8 *a0) {
    if (func_00343060(a0 + 0x298) == 0) {
        func_00335E68(a0 + 0x1F8, 0);
    } else {
        int *v = *(int **)(a0 + 0x288);
        func_00335E38(a0 + 0x1F8, D_001AE210 + *(float *)v,
                      D_001AE214 + *(float *)((u8 *)v + 4), 0.0f, 0.0f);
    }
}
