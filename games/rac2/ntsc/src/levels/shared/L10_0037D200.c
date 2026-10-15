#include "include_asm.h"

typedef unsigned char u8;

typedef struct { int status; u8 reserved[36]; } GornStatusRecord156;

extern GornStatusRecord156 D_L10_002A3C70[];

INCLUDE_ASM("asm/levels", func_L10_0037D200);

/* Report whether any configured status in the selected eleven records is nonzero */
int func_L10_0037F510(void)
{
    if (D_L10_002A3C70[2].status != 0 ||
        D_L10_002A3C70[3].status != 0 ||
        D_L10_002A3C70[4].status != 0 ||
        D_L10_002A3C70[5].status != 0 ||
        D_L10_002A3C70[8].status != 0 ||
        D_L10_002A3C70[9].status != 0 ||
        D_L10_002A3C70[10].status != 0 ||
        D_L10_002A3C70[11].status != 0 ||
        D_L10_002A3C70[12].status != 0 ||
        D_L10_002A3C70[13].status != 0 ||
        D_L10_002A3C70[14].status != 0)
        return 1;
    return 0;
}

INCLUDE_ASM("asm/levels", func_L10_0039D5E0);

extern void func_L10_0039D760(void);
extern void func_L00_002E6EF8(void *, void *);
extern u8 D_L10_0039D9C0[];

/* Refreshes and then hands the constructor at 0x0039D9C0 to the callback list. */
void func_L10_0039D728(void *a0) {
    func_L10_0039D760();
    func_L00_002E6EF8(D_L10_0039D9C0, a0);
}

INCLUDE_ASM("asm/levels", func_L10_0039D760);

INCLUDE_ASM("asm/levels", func_L10_0039D9C0);

INCLUDE_ASM("asm/levels", func_L10_0039DA48);

INCLUDE_ASM("asm/levels", func_L10_00418D88);

INCLUDE_ASM("asm/levels", func_L10_004190F0);

INCLUDE_ASM("asm/levels", func_L10_004191E8);

INCLUDE_ASM("asm/levels", func_L10_00419348);

INCLUDE_ASM("asm/levels", func_L10_004235A0);

INCLUDE_ASM("asm/levels", func_L10_00423698);
