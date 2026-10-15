#include "include_asm.h"

extern unsigned char *D_L00_001B0CA0 __attribute__((sda));

extern int D_L00_001B2C14 __attribute__((sda));

extern int *D_L00_001B0DB0 __attribute__((sda));

int func_L00_00400F38(char *arg)
{
    unsigned char *p = D_L00_001B0CA0;
    if (!p) {
        return 0;
    }
    return (unsigned int)(p[32] - 5) < 3;
}

int func_L00_00401E28(void)
{
    if (!D_L00_001B0CA0) {
        return 0;
    }
    return D_L00_001B2C14;
}

void func_L00_00402DB8(char *arg)
{
    int *p = D_L00_001B0DB0;
    if (p) {
        int *q = (int *)p[26];
        q[10] = 1;
    }
}

extern void func_L00_00402DB8(char *arg);

void func_L00_004028C0(char *arg, int value)
{
    unsigned char *p = D_L00_001B0CA0;
    int *o;

    if (!p) {
        return;
    }
    o = *(int **)(p + 104);
    p[32] = (arg != 0) ? 5 : 7;
    func_L00_00402DB8(arg);
    if (value != -1) {
        o[24] = value;
    } else {
        o[24] = 10;
    }
}

extern int D_L00_001B19C8_1[] __attribute__((sda));
extern char *D_L00_001B1EA8_1[] __attribute__((sda));
extern float D_L00_001B0C90 __attribute__((sda));

extern float func_002EE2B0(char *a, char *b);
extern void func_00381F80(char *a, char *b);

/* walks the object list once the shared flag is set, stopping at either special state and
   otherwise acting on the entries whose handle reports type 0x13 */
void func_L00_00401D50(char *arg)
{
    char *o;

    if ((D_L00_001B19C8_1[0] & 1) == 0)
        return;

    for (o = D_L00_001B1EA8_1[0]; o != 0; o = *(char **)(o + 40)) {
        unsigned char state = *(unsigned char *)(o + 32);

        if (state == 0xFE)
            return;
        if (state == 0xFD)
            return;
        if (o == 0 || *(int *)(o + 36) == 0 || *(short *)(*(int *)(o + 36) + 70) != 0x13)
            continue;
        if (func_002EE2B0(o + 16, arg + 16) < D_L00_001B0C90)
            func_00381F80(o, arg);
    }
}
