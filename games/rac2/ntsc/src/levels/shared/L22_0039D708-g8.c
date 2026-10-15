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

/* ---- 0x003A48F8 / 0x003A4C68: the same scan of the small-data table at
 * $gp+0x3C0, bounded by the count at 0x001B2AD4 (absolute, hence `nosda`).
 * No guard in the source: retail's leading `blez` is the loop's own entry
 * test, which is what splits the count between $v0 (the load) and $a2 (the
 * loop's bound copy). ---- */

extern int D_L22_001B2AD4 __attribute__((nosda));
extern int D_L22_001AF3B0[1] __attribute__((sda));

int func_L22_003A48F8(int key) {
    int i;
    for (i = 0; i < D_L22_001B2AD4; i++) {
        if (D_L22_001AF3B0[i] == key) return 0;
    }
    return 1;
}

/* The same scan, looking for the second argument and reporting the opposite
 * way. */
int func_L22_003A4C68(int unused, int key) {
    int i;
    for (i = 0; i < D_L22_001B2AD4; i++) {
        if (D_L22_001AF3B0[i] == key) return 1;
    }
    return 0;
}
