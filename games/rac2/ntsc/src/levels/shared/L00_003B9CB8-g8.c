#include "include_asm.h"

extern int D_L00_001AFDA0 __attribute__((sda));

/* Reads the small counter in the shared data area and reports whether it equals two. */
int func_L00_003BB5F0(void)
{
    return D_L00_001AFDA0 == 2;
}

extern char D_L00_0023DDA0[] __attribute__((nosda));

extern char D_L00_0023F060[] __attribute__((nosda));

/* Clears one word of each of the thirty 0xA0-byte records that start at D_L00_0023DDA0,
   then one word of each of the sixteen 0xB0-byte records at D_L00_0023F060, walking down
   from the last record of each table, and finally the shared flag word. */
void func_L00_003BB600(void)
{
    char *base;
    char *p;
    int i;

    base = D_L00_0023DDA0;
    p = base + (0x1D * 0xA0 + 0x8C);
    i = 0x1D;
    do {
        *(int *)p = 0;
        i--;
        p -= 0xA0;
    } while (i >= 0);

    base = D_L00_0023F060;
    p = base + (0xF * 0xB0 + 0xA0);
    i = 0xF;
    do {
        *(int *)p = 0;
        i--;
        p -= 0xB0;
    } while (i >= 0);

    D_L00_001AFDA0 = 0;
}

extern int D_L04_001A8F00;

extern int D_L00_001AFF44 __attribute__((sda));

extern int D_L00_001AFF98 __attribute__((sda));

/* Clears the shared flag word this file's small-data area holds at 0x1AFF98. */
void func_L00_003C58E0(void)
{
    D_L00_001AFF98 = 0;
}

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
