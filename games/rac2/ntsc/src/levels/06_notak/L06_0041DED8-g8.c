#include "include_asm.h"
#include "common.h"

extern char *D_L06_001B281C __attribute__((sda));

extern u8 *D_L06_001FB3A0[];

void func_L06_0041F798(u8 *arg0)
{
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L06_001FB3A0[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = D_L06_001B281C;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xBBA) {
                *(int *)(*(char **)(q + 0x68) + 0x258) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

void func_L06_00443E88(u8 *arg0)
{
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L06_001FB3A0[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = D_L06_001B281C;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xDB0) {
                *(short *)(*(int *)(q + 0x68) + 0x316) = 1;
            }
        }
        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xBBA) {
                *(int *)(*(char **)(q + 0x68) + 0x258) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}
