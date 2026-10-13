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

INCLUDE_ASM("asm/game", func_0028E260);

INCLUDE_ASM("asm/game", func_0028E2A8);
