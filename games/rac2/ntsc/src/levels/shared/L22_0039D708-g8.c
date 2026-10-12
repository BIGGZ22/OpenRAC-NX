#include "include_asm.h"

/* Small-data file: these bodies reach their globals through $gp / the
 * one-instruction model, so they only reproduce at -G8. */

extern unsigned char D_L22_001B2A90[] __attribute__((sda));
extern int *D_L22_001B2AB4[4] __attribute__((sda));
extern int D_L22_001AF314 __attribute__((sda));

extern void ref_L22_003A1E50_00(char *, char *);
extern void ref_L22_003A1E50_01(void *);
extern void ref_L22_003A3C70_00(void *, int, int, int);

void func_L22_003A1E50(char *a0) {
    char *sub = *(char **)(a0 + 0x68);
    unsigned short *counter;
    ref_L22_003A1E50_00(a0, sub + 0x50);
    counter = (unsigned short *)(a0 + 0x96);
    if (*counter <= 0xFFFE) (*counter)++;
    if (*(int *)(sub + 0x37C) != 0) *(int *)(sub + 0x37C) = *(int *)(sub + 0x37C) - 1;
    ref_L22_003A1E50_01(sub + 0x464);
    ref_L22_003A1E50_01(D_L22_001B2A90);
}

void func_L22_003A2A08(void) {
    int *p = D_L22_001B2AB4[0];
    if (p[3] > 0) p[3] = p[3] - 1;
}

void func_L22_003A3C70(int a0, int a1) {
    ref_L22_003A3C70_00(D_L22_001AF314, a0, a1, 0);
}
