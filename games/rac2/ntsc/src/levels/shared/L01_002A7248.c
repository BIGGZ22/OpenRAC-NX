#include "include_asm.h"

typedef struct { char *p[6]; } Sep6;

extern Sep6 D_L01_001AA550;

extern char D_L01_001AA568[];

extern char D_L01_001AA578[];

extern char D_L01_001AA588[];

extern int func_00115DA8_2(char *dst, char *fmt, ...);

INCLUDE_ASM("asm/levels", func_L01_002A7248);

INCLUDE_ASM("asm/levels", func_L01_002A84D8);

INCLUDE_ASM("asm/levels", func_L01_002A8C48);

INCLUDE_ASM("asm/levels", func_L01_002A9298);

INCLUDE_ASM("asm/levels", func_L01_002ABF90);

INCLUDE_ASM("asm/levels", func_L01_002AE128);

INCLUDE_ASM("asm/levels", func_L01_002B0318);

INCLUDE_ASM("asm/levels", func_L01_002B3458);

INCLUDE_ASM("asm/levels", func_L01_002B35E8);

INCLUDE_ASM("asm/levels", func_L01_002B3788);

INCLUDE_ASM("asm/levels", func_L01_002B3D28);

INCLUDE_ASM("asm/levels", func_L01_002B46A0);

INCLUDE_ASM("asm/levels", func_L01_002B4DA0);

INCLUDE_ASM("asm/levels", func_L01_002B5EA0);

INCLUDE_ASM("asm/levels", func_L01_002B6948);

INCLUDE_ASM("asm/levels", func_L01_002BE0E0);

INCLUDE_ASM("asm/levels", func_L01_002C1988);

INCLUDE_ASM("asm/levels", func_L01_002C2CF0);

INCLUDE_ASM("asm/levels", func_L01_002C81D0);

INCLUDE_ASM("asm/levels", func_L01_002C8990);

INCLUDE_ASM("asm/levels", func_L01_002C89A8);

INCLUDE_ASM("asm/levels", func_L01_002C98B8);

INCLUDE_ASM("asm/levels", func_L01_002E1150);

INCLUDE_ASM("asm/levels", func_L01_002E1510);

INCLUDE_ASM("asm/levels", func_L01_002E1670);

INCLUDE_ASM("asm/levels", func_L01_002E1680);

INCLUDE_ASM("asm/levels", func_L01_002E2608);

INCLUDE_ASM("asm/levels", func_L01_002EABA8);

INCLUDE_ASM("asm/levels", func_L01_002F0E88);

INCLUDE_ASM("asm/levels", func_L01_002F14E8);

INCLUDE_ASM("asm/levels", func_L01_002F1588);

INCLUDE_ASM("asm/levels", func_L01_002F1888);

INCLUDE_ASM("asm/levels", func_L01_002F1F70);

INCLUDE_ASM("asm/levels", func_L01_002F1FD0);

INCLUDE_ASM("asm/levels", func_L01_00304270);

INCLUDE_ASM("asm/levels", func_L01_00305E18);

INCLUDE_ASM("asm/levels", func_L01_00310BC0);

INCLUDE_ASM("asm/levels", func_L01_003115E8);

INCLUDE_ASM("asm/levels", func_L01_00311C18);

INCLUDE_ASM("asm/levels", func_L01_003120A8);

INCLUDE_ASM("asm/levels", func_L01_003128C0);

INCLUDE_ASM("asm/levels", func_L01_003161F0);

INCLUDE_ASM("asm/levels", func_L01_003165E8);

INCLUDE_ASM("asm/levels", func_L01_00316C48);

INCLUDE_ASM("asm/levels", func_L01_00318638);

INCLUDE_ASM("asm/levels", func_L01_0031A278);

INCLUDE_ASM("asm/levels", func_L01_0031AB20);

INCLUDE_ASM("asm/levels", func_L01_0031B300);

INCLUDE_ASM("asm/levels", func_L01_0031BC68);

INCLUDE_ASM("asm/levels", func_L01_0031EF98);

INCLUDE_ASM("asm/levels", func_L01_0031F080);

INCLUDE_ASM("asm/levels", func_L01_003200C8);

INCLUDE_ASM("asm/levels", func_L01_003208D0);

INCLUDE_ASM("asm/levels", func_L01_003209F8);

INCLUDE_ASM("asm/levels", func_L01_00321178);

INCLUDE_ASM("asm/levels", func_L01_00322288);

INCLUDE_ASM("asm/levels", func_L01_00322358);

INCLUDE_ASM("asm/levels", func_L01_00323310);

/* shared native body of family d1b66aa7f0ecd20d */
void func_L01_003242C8(char *dst, int x, int idx)
{
    Sep6 t;
    t = D_L01_001AA550;
    if (x > 999999) {
        int m = x / 1000000;
        int w = x % 1000000;
        func_00115DA8_2(dst, D_L01_001AA568, m, t.p[idx % 6], w / 1000,
                          t.p[idx % 6], w % 1000);
    } else if (x >= 1000)
        func_00115DA8_2(dst, D_L01_001AA578, x / 1000,
                          t.p[idx % 6], x % 1000);
    else
        func_00115DA8_2(dst, D_L01_001AA588, x);
}

INCLUDE_ASM("asm/levels", func_L01_00324408);

INCLUDE_ASM("asm/levels", func_L01_00324410);

INCLUDE_ASM("asm/levels", func_L01_00324550);

INCLUDE_ASM("asm/levels", func_L01_003245E0);

INCLUDE_ASM("asm/levels", func_L01_003246A0);

INCLUDE_ASM("asm/levels", func_L01_003246D0);

INCLUDE_ASM("asm/levels", func_L01_003246F8);

INCLUDE_ASM("asm/levels", func_L01_00326DD8);

INCLUDE_ASM("asm/levels", func_L01_0032EF38);

INCLUDE_ASM("asm/levels", func_L01_00331CA0);

INCLUDE_ASM("asm/levels", func_L01_00331E08);

INCLUDE_ASM("asm/levels", func_L01_003377C8);

INCLUDE_ASM("asm/levels", func_L01_00337888);

INCLUDE_ASM("asm/levels", func_L01_00337A60);

INCLUDE_ASM("asm/levels", func_L01_00337B80);

INCLUDE_ASM("asm/levels", func_L01_0034EA48);

INCLUDE_ASM("asm/levels", func_L01_00355BA0);

INCLUDE_ASM("asm/levels", func_L01_00367F78);

INCLUDE_ASM("asm/levels", func_L01_00371228);

INCLUDE_ASM("asm/levels", func_L01_003801B8);

INCLUDE_ASM("asm/levels", func_L01_003808C0);

INCLUDE_ASM("asm/levels", func_L01_0038EF40);

INCLUDE_ASM("asm/levels", func_L01_003907B8);

INCLUDE_ASM("asm/levels", func_L01_003909E0);

INCLUDE_ASM("asm/levels", func_L01_00390A50);

INCLUDE_ASM("asm/levels", func_L01_00390C60);

INCLUDE_ASM("asm/levels", func_L01_00394148);

INCLUDE_ASM("asm/levels", func_L01_00394240);
