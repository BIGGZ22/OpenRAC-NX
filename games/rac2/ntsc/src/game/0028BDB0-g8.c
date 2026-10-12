#include "include_asm.h"
typedef unsigned char u8;

extern void func_0028B540(void);
extern int D_001A8FB0 __attribute__((sda));
extern int D_001A8FB8;
extern int D_001A8FB4;
void func_0028E408(void) {
    func_0028B540();
    D_001A8FB0 = 1;
    D_001A8FB8 = -1;
    D_001A8FB4 = 0;
}
