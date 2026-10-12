#include "include_asm.h"

struct ResetBlock {
    long long first;
    int second;
};

void func_L00_002EE0C0(void *a0) {
    struct ResetBlock *p = (struct ResetBlock *)a0;
    p->second = 0;
    p->first = 0;
}
