#include "include_asm.h"
typedef unsigned char u8;
typedef int s32;

extern int D_L18_001B15C0 __attribute__((sda));
extern int D_L18_001B15B0 __attribute__((sda));
extern float D_L18_001B15B8 __attribute__((sda));
extern float D_L18_001B15B4 __attribute__((sda));

extern int D_L18_001B15E0 __attribute__((sda));
extern int D_L18_001B15D0 __attribute__((sda));
extern float D_L18_001B15D8 __attribute__((sda));
extern float D_L18_001B15D4 __attribute__((sda));

extern int D_L18_001B1620 __attribute__((sda));
extern int D_L18_001B1614 __attribute__((sda));
extern float D_L18_001B161C __attribute__((sda));
extern float D_L18_001B1618 __attribute__((sda));
extern void func_0029FC20(s32 arg0, s32 arg1, void *arg2);
extern void func_002830C0(void *arg0, void *arg1, void *arg2);
extern void func_00282CC0(void *arg0, void *arg1, void *arg2);
extern void func_L00_00325AE8(void *arg0, s32 arg1, float arg2, float arg3);

void func_L18_00432FB8(u8 *a0) {
    u8 buf[0x50];
    u8 *s0 = buf + 0x10;
    func_0029FC20(*(s32 *)(*(u8 **)(a0 + 0x68) + 0x7C0), 1, s0);
    func_002830C0(buf, &D_L18_001B15C0, s0);
    func_00282CC0(buf, buf, buf + 0x40);
    func_L00_00325AE8(buf, D_L18_001B15B0, D_L18_001B15B8, D_L18_001B15B4);
}
void func_L18_00433020(u8 *a0) {
    u8 buf[0x50];
    u8 *s0 = buf + 0x10;
    func_0029FC20(*(s32 *)(*(u8 **)(a0 + 0x68) + 0x7C0), 1, s0);
    func_002830C0(buf, &D_L18_001B15E0, s0);
    func_00282CC0(buf, buf, buf + 0x40);
    func_L00_00325AE8(buf, D_L18_001B15D0, D_L18_001B15D8, D_L18_001B15D4);
}
void func_L18_00433168(u8 *a0, s32 a1) {
    u8 buf[0x50];
    u8 *s0 = buf + 0x10;
    func_0029FC20(*(s32 *)(*(u8 **)(a0 + 0x68) + 0x7C4), a1, s0);
    func_002830C0(buf, &D_L18_001B1620, s0);
    func_00282CC0(buf, buf, buf + 0x40);
    func_L00_00325AE8(buf, D_L18_001B1614, D_L18_001B161C, D_L18_001B1618);
}
