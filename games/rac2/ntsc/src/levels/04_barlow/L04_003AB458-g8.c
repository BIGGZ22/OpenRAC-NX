#include "include_asm.h"

/* Small data reached through $gp: only one instruction at -G8. */
extern int D_L04_001B0710 __attribute__((sda));
extern int D_L04_001B0714 __attribute__((sda));
extern int D_L04_001B0718 __attribute__((sda));

extern char func_L04_00406C00[];

extern void func_002A0488(void *, int, int, int);
extern void func_L00_002E6CF0(void *, void *);

extern int D_L04_001BFAD8 __attribute__((nosda));
extern int D_0018C0B0 __attribute__((nosda));
extern char D_L04_001A8F20[1];
extern int D_L04_001B0CF8 __attribute__((sda));

extern void func_002A8C28(void *, void *, int, int, float, float, float, int, int);
extern int func_00277258(void *, int, float, void *);
extern void func_002AB250(void *owner, char *offset, void *filter);
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

void func_L04_00422240(char *a0)
{
    char *s0 = a0;
    char *s1 = s0 + 0x10;

    func_002A8C28(s0, s1, 0x10001, 5, 1.0f, 1.0f, 1.0f, 1, 0);
    if (func_00277258(s1, 1, 1.0f, s0) != 0) {
        if (D_L04_001BFAD8 != 0 && D_L04_001BFAD8 == D_0018C0B0) {
            if (D_L04_001B0CF8 <= 0) {
                D_L04_001B0CF8 = 0x3C;
            }
        }
    }
    if (D_L04_001B0CF8 > 0) {
        func_002AB250(D_0018C0B0, D_L04_001A8F20, 0xFFFF);
        func_00282948(&D_L04_001B0CF8);
    }
}
