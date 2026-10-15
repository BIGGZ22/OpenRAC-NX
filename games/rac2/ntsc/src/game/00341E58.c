#include "include_asm.h"

extern int func_00341E90(char *);
int func_00341E58(char *object) { int index = func_00341E90(object); return *(int *)(object + 0xD8) + index * 20; }
