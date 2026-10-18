/* functional: high. written from the US assembly (PAL's func_001263A8 has no C): same behaviour as the retail function, tail calls kept as calls */
#include "types.h"
#include "rnc/sdk/libmpeg.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
extern u8 D_00153770[];
extern s32 _Error();
extern void func_001271D8();
extern void func_00127178();

void _doMC(struct MpegDecoder *mpeg, s32 arg1) {
    u8 *d = (u8 *)mpeg;
    s32 off = arg1 * 0x140;
    s32 i;

    if (*(s32 *)(d + off + 0x6C8) != 0) {
        for (i = 0; *(s32 *)(d + 0x6BC + off) > i; i++) {
            u8 *p = d + off + 0x590;
            ((void (*)(u8 *))*(s32 *)(d + 0x5B8 + i * 4 + off))(p + i * 0x1C + 0x48);
            ((void (*)(u8 *))*(s32 *)(d + 0x5C8 + i * 4 + off))(p + i * 0x1C + 0xB8);
        }
    }
    if (*(s32 *)(d + 0x6C0 + off) != 0 && *(s32 *)(d + off + 0x6CC) != 0) {
        _Error(mpeg, D_00153770);
    }
    if (*(s32 *)(d + 0x6C0 + off) != 0) {
        func_001271D8(*(s32 *)(d + 0x6B8 + off), *(s32 *)(d + off + 0x594));
    } else if (*(s32 *)(d + off + 0x6CC) != 0) {
        func_001271D8(*(s32 *)(d + 0x6B8 + off), *(s32 *)(d + 0x81C));
    } else {
        func_00127178(*(s32 *)(d + 0x6B8 + off), *(s32 *)(d + 0x81C), *(s32 *)(d + off + 0x594));
    }
}
