#include "include_asm.h"

extern int D_L07_001B0AE0 __attribute__((sda));

extern void func_0029F338(int);

/* Run the shared per-object pass once a level latch is set. */
void func_L07_003E61B8(int arg)
{
    if (D_L07_001B0AE0 != 0)
        func_0029F338(arg);
}
