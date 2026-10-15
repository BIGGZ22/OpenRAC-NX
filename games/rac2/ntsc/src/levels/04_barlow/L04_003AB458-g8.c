#include "include_asm.h"

/* Small data reached through $gp: only one instruction at -G8. */
extern int D_L04_001B0710 __attribute__((sda));
extern int D_L04_001B0714 __attribute__((sda));
extern int D_L04_001B0718 __attribute__((sda));

extern char func_L04_00406C00[];

extern void func_002A0488(void *, int, int, int);
extern void func_L00_002E6CF0(void *, void *);

extern int D_L04_001BFAD8;
extern int D_L04_0018C0B0;
extern char D_L04_001A8F20[];
extern int D_L04_001B0CF8 __attribute__((sda));

extern void func_002A8C28(void *, void *, int, int, float, float, float, int, int);
extern int func_00277258(void *, int, void *, float);
extern int func_002AB250(void *, int, int);
extern int func_00282948(void *);

extern int D_L04_001B07B0 __attribute__((sda));
extern int D_L04_001B07B4 __attribute__((sda));
extern int D_L04_001B07B8 __attribute__((sda));
extern char D_L04_001B9680[];
extern char func_L04_00407B80[];
extern void func_L00_002E6EF8(void *, void *);

void func_L04_00407000(char *a0)
{
    char *s0 = a0;

    if (*(unsigned char *)(s0 + 0x20) == 0) {
        func_002A0488(s0, D_L04_001B0710, D_L04_001B0714, D_L04_001B0718);
        *(unsigned char *)(s0 + 0x20) = 1;
    } else {
        func_002A0488(s0, D_L04_001B0710, D_L04_001B0714, D_L04_001B0718);
        func_L00_002E6CF0(func_L04_00406C00, s0);
        *(unsigned short *)(s0 + 0x34) = (*(unsigned short *)(s0 + 0x34) & ~1) | 1;
        *(unsigned short *)(s0 + 0x32) = 0;
    }
}

extern int D_L04_001B24DC __attribute__((sda));
extern char D_L04_001FB1E0[];
