#include "include_asm.h"

extern void *D_L00_00232A60[24];

extern void func_0029F338(void *arg);

void func_L00_00384C20(char *obj)
{
    void **p = (void **)(*(char **)(obj + 0x68) + 0x48);
    int i = 1;
    do {
        int v = (int)*p;
        p++;
        i--;
        if (v) {
            func_0029F338((void *)v);
        }
    } while (i >= 0);
    p = D_L00_00232A60;
    i = 23;
    do {
        int v = (int)*p;
        p++;
        i--;
        if (v) {
            func_0029F338((void *)v);
        }
    } while (i >= 0);
}
