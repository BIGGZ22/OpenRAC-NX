#include "include_asm.h"

extern int D_L00_001AFDA0 __attribute__((sda));

/* Reads the small counter in the shared data area and reports whether it equals two. */
int func_L00_003BB5F0(void)
{
    return D_L00_001AFDA0 == 2;
}

extern int D_L04_001A8F00;

extern int D_L00_001AFF44 __attribute__((sda));

extern char D_0018C0B0[];

extern float func_L00_0037E668(void);

extern int func_L00_003C5B18(float f12);

extern void func_0029FCC0(char *a, int b, char *c);

extern void func_L00_003C48D0(char *p);

extern void func_L00_003C4B98(char *p);

extern void func_L00_003C4F78(char *p);

/* Gate followed by a fixed sequence of four bound helpers on the state record at offset 0x68. */
void func_L00_003C4850(char *p)
{
    char *q;

    if (D_L04_001A8F00 != 0)
        return;
    q = *(char **)(p + 0x68);
    if (func_L00_003C5B18(func_L00_0037E668()) == 0)
        return;
    if (D_L00_001AFF44 == 0x28)
        return;
    func_0029FCC0(*(char **)D_0018C0B0, 0xA, q + 0x30);
    func_L00_003C48D0(p);
    func_L00_003C4B98(p);
    func_L00_003C4F78(p);
}
