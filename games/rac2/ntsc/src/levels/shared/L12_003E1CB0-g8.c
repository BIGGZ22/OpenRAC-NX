#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

extern u16 D_L12_001B0B0C __attribute__((sda));

void func_L12_003E2448(u8 *a0) {
    *(u8 *)(a0 + 0x95) = 9;
    *(u16 *)(*(u32 *)(a0 + 0x68) + 0x29E) = D_L12_001B0B0C;
}
