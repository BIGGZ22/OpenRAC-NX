#include "include_asm.h"

int func_00341E58(char *object) {
    extern void func_00341E90(char *);
    int (*get_index)(char *) = (int (*)(char *))func_00341E90;
    int index = get_index(object);
    return *(int *)(object + 0xD8) + index * 20;
}
