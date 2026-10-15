#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

/* Small-data controls: the four measured globals the level's script hooks share. */
extern s32 D_L11_001B1530 __attribute__((sda));

extern s32 D_L11_001B1540 __attribute__((sda));

extern s32 D_L11_001B1550 __attribute__((sda));

extern s32 D_L11_001B1560 __attribute__((sda));

extern void func_3004A8(void *);

extern void func_4882F0(void *, void *, s32, s32, s32);

extern void func_488268(s32, s32);

extern void func_4881A8(void *);

extern void func_488200(void *);

extern u32 D_L11_001B32E8[];

extern u8 D_L11_001BA8C0[];

extern u8 *D_L11_001B359C __attribute__((sda));

extern u8 *D_L11_001FC420[];

extern float func_282E48(void *, void *);

extern void func_32C9E0();

extern void func_440C90(u8 *);

/* arms the script bridge over the measured buffers and starts it on the shared slot */
void func_L11_00443380(void) {
    s32 buffer[4];
    func_3004A8(buffer);
    func_4882F0(&D_L11_001B1550, &D_L11_001B1560, 1, 0, 0);
    func_488268(2, 0x168);
    func_4881A8(&D_L11_001B1530);
    func_488200(&D_L11_001B1540);
}

