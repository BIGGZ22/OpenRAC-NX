/* Best plain-C attempt measured under the -G8 sibling profile. Last check: 304 bytes vs 280; 0/27. */
#include "include_asm.h"
typedef unsigned char u8;
extern int D_L00_001A903C;
extern int D_L00_001A7F8C __attribute__((sda));
extern u8 D_001A7BE0;
extern u8 D_001A7A60[] __attribute__((sda));
extern int func_00282948(char *);
extern int func_002B0E90(void);
extern int func_L00_002F43D0(void);
extern void func_002B0A18(int, int);

void func_L00_002B2200(void)
{
    int result;
    int count;
    int selector;
    int flag;

    if (D_L00_001A903C == 0)
        return;
    if (func_00282948((char *)&D_L00_001A7F8C) == 0)
        return;
    count = func_002B0E90();
    selector = func_L00_002F43D0();
    flag = D_001A7BE0;
    if (flag != 0)
        result = 3;
    else
        result = 7;
    if (flag == 0 && selector <= 0)
        func_002B0A18(0x1B8C, -1);
    else if (count == result || count == 10 || count == 15 || count == 19 || count == 25 || count == 29)
        func_002B0A18(0x1B8C, -1);
    if (count == 7 || count == 26)
        func_002B0A18(0x1B8D, -1);
    if (count == 7 || count == 20 || count == 30)
        func_002B0A18(0x1B8F, -1);
    if (D_001A7A60[0] != 0)
        func_002B0A18(0x1B8F, -1);
    D_L00_001A903C = 0;
    D_L00_001A7F8C = 300;
}
