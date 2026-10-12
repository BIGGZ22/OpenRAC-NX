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

INCLUDE_ASM("asm/game", func_0028DF38);

INCLUDE_ASM("asm/game", func_0028DF68);

INCLUDE_ASM("asm/game", func_0028DF70);

INCLUDE_ASM("asm/game", func_0028DF80);

INCLUDE_ASM("asm/game", func_0028E138);

INCLUDE_ASM("asm/game", func_0028E260);

INCLUDE_ASM("asm/game", func_0028E2A8);

INCLUDE_ASM("asm/game", func_0028E408);
