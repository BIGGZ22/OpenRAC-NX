#include "include_asm.h"

extern int D_L05_00231E40_1[][4];

typedef unsigned char u8;

typedef int s32;

typedef unsigned int u32;

typedef struct {
    u32 field00,field04;
    s32 field08;
    u8 gap0c[4];
} OrbitalPairTableView16;

extern u8 D_L00_00231340[];

/* shared native body of family 954231c412 */
int func_L00_0037DD80(int a0, int a1)
{
    int i;
    int r = 1;

    for (i = 0; i < 32; i++)
        if (D_L05_00231E40_1[i][1] == a0 && D_L05_00231E40_1[i][0] == a1) {
            r = 0;
            break;
        }
    return r;
}

/* searches 32 sixteen-byte records for matching key and owner words and returns the value word or minus one */
s32 func_L00_0037DDE8(u32 key,u32 owner) {
    OrbitalPairTableView16 *entry=(OrbitalPairTableView16 *)D_L00_00231340;
    s32 checked=0;
    do {
        ++checked;
        if(entry->field04==key && entry->field00==owner) return entry->field08;
        ++entry;
    } while(checked<32);
    return -1;
}

INCLUDE_ASM("asm/levels", func_L00_0037DE30);

INCLUDE_ASM("asm/levels", func_L00_0037DEF8);

INCLUDE_ASM("asm/levels", func_L00_0037E000);

INCLUDE_ASM("asm/levels", func_L00_0037E0C0);

INCLUDE_ASM("asm/levels", func_L00_0037E418);

INCLUDE_ASM("asm/levels", func_L00_0037E5C0);

INCLUDE_ASM("asm/levels", func_L00_0037E5F0);

INCLUDE_ASM("asm/levels", func_L00_0037E640);
