#include "include_asm.h"

#include "common.h"

extern float D_L13_001B1310 __attribute__((sda));

extern int D_L13_001B1300 __attribute__((sda));

extern int D_L13_001B1304 __attribute__((sda));

extern int D_L13_001B1308 __attribute__((sda));

extern int D_L13_001B130C __attribute__((sda));

extern float D_L13_001B1314 __attribute__((sda));

extern char *func_003249F0(int, int);

extern float func_00324B10(void);

extern void func_L13_0040DFC0(char *, char *, char *, char *, int, char *, float, float,
                              float);

/* draws the two halves of the sign band, each from its own colour pair */
INCLUDE_ASM("asm/levels", func_L13_0040DE50);
