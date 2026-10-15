#include "include_asm.h"

void func_L00_00413298(char *object)
{
    char *state = *(char **)(object + 0x68);
    int i;

    for (i = 0; i < 10; i++) {
        char *entry = *(char **)(state + 0) + i * 16;
        *(int *)(entry + 0) = 0;
        *(int *)(entry + 4) = 0;
        *(int *)(entry + 8) = 0;
        *(float *)(entry + 12) = 1.0f;
    }

    for (i = 0; i < 20; i++) {
        char *entry = *(char **)(state + 4) + i * 16;
        *(int *)(entry + 0) = 0;
        *(int *)(entry + 4) = 0;
        *(int *)(entry + 8) = 0;
        *(float *)(entry + 12) = 1.0f;
    }

    *(int *)(state + 8) = 0;
    *(short *)(state + 0x1C) = 6;
    *(int *)(state + 0xC) = 0;
    *(int *)(state + 0x10) = 0;
    *(int *)(state + 0x14) = 0;
}
