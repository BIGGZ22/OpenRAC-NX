#include "include_asm.h"

/* Functions whose retail body reaches level data through $gp (small data area). */
extern void func_2EE5B0(void *, void *, void *);
extern void func_2EE108(void *, void *, void *);

extern int D_L00_001B0BE0 __attribute__((sda));

void func_L00_003FE280(char *a0, char *a1)
{
    func_2EE5B0(a1, &D_L00_001B0BE0, a0 + 0xC0);
    func_2EE108(a1, a1, a0 + 0x10);
}
