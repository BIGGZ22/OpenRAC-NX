#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned short u16;

typedef int s32;

extern void func_0028BCE8(void);

/* authored C for independently verified level-native body */
void func_0028BDB0(u8 *object) {
 func_0028BCE8();
 *(s32 *)(object+0x58)=210;*(s32 *)(object+0x5c)=200;
 *(s32 *)(object+0x74)=-2;*(s32 *)(object+0x78)=30;
 *(u16 *)(object+0x48)=0;*(u16 *)(object+0x4a)=0;*(s32 *)(object+0x70)=0;
}

INCLUDE_ASM("asm/game", func_0028BE00);

INCLUDE_ASM("asm/game", func_0028C6D0);

INCLUDE_ASM("asm/game", func_0028CC98);

INCLUDE_ASM("asm/game", func_0028CCE0);

INCLUDE_ASM("asm/game", func_0028D1E8);

INCLUDE_ASM("asm/game", func_0028DDD8);

void func_0028DF38(char *object) { extern void func_0028B950(u8 *); *(int *)(object + 0x58) = 0x20; *(int *)(object + 0x7C) = 0x96; *(int *)(object + 0x5C) = 0x20; func_0028B950((u8 *)object); }

int func_0028DF68(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_0028DF70);

INCLUDE_ASM("asm/game", func_0028DF80);

INCLUDE_ASM("asm/game", func_0028E138);

typedef struct { char reserved00[0x1248]; int selected; char reserved124C[0x1068]; unsigned char blocked; } MapSelection;
typedef struct { char reserved00[0x88]; unsigned short valid; char reserved8A[0x56]; } MapRecord;
extern MapSelection D_00189E20;
extern unsigned char D_00139568[];
extern MapRecord ref_L00_0028E260_00[];
int func_0028E260(void)
{
    int selected = D_00189E20.selected;
    int blocked = D_00189E20.blocked;
    unsigned int index = D_00139568[selected];
    if (((MapRecord *)(index * 0xE0 + (unsigned int)ref_L00_0028E260_00))->valid == 0) selected = 0;
    if (blocked) selected = 0;
    return selected;
}

INCLUDE_ASM("asm/game", func_0028E2A8);
