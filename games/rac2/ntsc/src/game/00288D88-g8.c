#include "include_asm.h"

extern int D_001A8D44 __attribute__((sda));
extern char *ref_L00_0028B338_00 __attribute__((sda));
extern char *D_001B1B5C;
extern char *D_001B1B60;
extern int D_001B1B64;
extern int D_001B1B6C;
extern char *D_001B18D8;
extern char *D_001B18DC;
extern int D_001B18E0;
extern int D_001B18E4;

void func_0028B338(int requested)
{
    int state = D_001A8D44;
    if (requested != state) {
        char *first;
        char *second;
        int third, fourth, active_fourth;
        char *active_first = ref_L00_0028B338_00;
        requested = state ^ 1;
        first = *(char *volatile *)&D_001B18D8;
        *(char *volatile *)&D_001B18D8 = active_first;
        active_first = *(char *volatile *)&D_001B1B60;
        second = *(char *volatile *)&D_001B18DC;
        *(char *volatile *)&D_001B18DC = active_first;
        active_first = (char *)*(volatile int *)&D_001B1B64;
        third = *(volatile int *)&D_001B18E0;
        active_fourth = *(volatile int *)&D_001B1B6C;
        fourth = *(volatile int *)&D_001B18E4;
        *(volatile int *)&D_001A8D44 = requested;
        *(char *volatile *)&D_001B1B5C = first;
        *(char *volatile *)&D_001B1B60 = second;
        *(volatile int *)&D_001B1B64 = third;
        *(volatile int *)&D_001B18E0 = (int)active_first;
        *(volatile int *)&D_001B1B6C = fourth;
        *(volatile int *)&D_001B18E4 = active_fourth;
    }
}
