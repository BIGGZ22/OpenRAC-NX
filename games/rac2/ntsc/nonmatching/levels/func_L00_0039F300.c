#include "include_asm.h"

/* Functions of L00_0038F178.c whose float constants the retail build reached through $gp. */

extern void func_002ACEC0(char *a, char *b, char *c, float value);

extern void func_00282F20(char *a, char *b, float value);

extern char D_L00_0023A670[];

extern char D_L00_0023A6F0[];

extern float D_L00_001AF448 __attribute__((sda));

extern float D_L00_001AF470 __attribute__((sda));

/* Fills eight 0x10-byte records: two helpers per record, the second one a quarter scale. */

void func_L00_0039F300(void)
{
    char *q;
    char *p;
    int i;

    p = D_L00_0023A670;
    q = D_L00_0023A6F0;
    for (i = 7; i >= 0; i--) {
        func_002ACEC0(p, p, q, D_L00_001AF448);
        q += 0x10;
        func_00282F20(p, p, D_L00_001AF470);
        p += 0x10;
    }
}
