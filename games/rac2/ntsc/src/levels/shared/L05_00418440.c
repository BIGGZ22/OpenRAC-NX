#include "include_asm.h"

extern void func_00282CC0(void *a, void *b, void *c);

extern float func_00282E48(char *a, char *b);

extern char D_L05_001B3510[];

extern char D_L05_001B34F0[];

extern float D_L05_001B3500;

void func_L05_00418440(char *object)
{
    char *state = *(char **)(object + 0x68);
    char *entry = *(char **)(state + 4);
    char *local;

    if (*(short *)(entry + 0xAA) != 0x957)
        return;
    local = object + 0x10;
    if (func_00282E48(local, D_L05_001B3510) > 15.0f)
        return;
    func_00282CC0(D_L05_001B34F0, D_L05_001B34F0, local);
    D_L05_001B3500 = D_L05_001B3500 + 1.0f;
}
