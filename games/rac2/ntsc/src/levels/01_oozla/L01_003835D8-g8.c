#include "include_asm.h"
#include "common.h"

extern float D_L01_001AF87C __attribute__((sda));

extern float D_L01_001AFE18 __attribute__((sda));

float func_L01_003BAFB0(void) {
    return D_L01_001AFE18;
}

extern char *D_L01_001B255C __attribute__((sda));

extern char D_L01_001BF930[];

extern u8 *D_L01_001FB020[];

extern void func_00282CC0(char *, char *, char *);

extern void func_00282CF0(char *, char *, char *);

extern void func_00282F20(char *, char *, float);

extern int func_00277258(char *, int, char *, float);

void func_L01_003A65B0(char *arg0) {
    char buf[16];

    func_00282F20(buf, arg0 + 0xE0, D_L01_001AF87C);
    func_00282CC0(buf, buf, arg0 + 0x10);
    if (func_00277258(buf, 0, arg0, D_L01_001AF87C) != 0) {
        func_00282CF0(buf, D_L01_001BF930, buf);
        func_00282CC0(arg0 + 0x10, arg0 + 0x10, buf);
    }
}

void func_L01_003B5290(u8 *arg0) {
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L01_001FB020[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = D_L01_001B255C;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0x9A2) {
                *(int *)(*(char **)(q + 0x68) + 0x350) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}
