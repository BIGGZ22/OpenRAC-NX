#include "include_asm.h"

extern volatile int D_001A7318;
extern char D_001AB658 __attribute__((sda));
extern char D_001AB628 __attribute__((sda));

int func_002CC528(void *object)
{
    void *alternate = &D_001AB628;
    void *selected = &D_001AB658;
    int enabled = D_001A7318;
    if (enabled)
        selected = alternate;
    *(void **)((char *)object + 0x34) = selected;
    return 0;
}
