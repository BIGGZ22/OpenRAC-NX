#include "include_asm.h"

typedef unsigned int u32;

typedef int s32;

extern s32 D_L24_001B1B70[] __attribute__((sda));

extern u32 D_L24_001B1B74[] __attribute__((sda));

typedef unsigned char u8;

INCLUDE_ASM("asm/game", func_002DDEC8);

INCLUDE_ASM("asm/game", func_002DE0D0);

INCLUDE_ASM("asm/game", func_002DE3A0);

INCLUDE_ASM("asm/game", func_002DE550);

INCLUDE_ASM("asm/game", func_002DE618);

INCLUDE_ASM("asm/game", func_002DE6F0);

/* authored C for independently verified level-native body */
s32 func_002DE750(s32 key) {
 s32 count=0;
 u32 *flags=D_L24_001B1B74;
 s32 *keys=D_L24_001B1B70;
 do {
  count++;
  if(*keys!=key) {flags+=2;keys+=2;continue;}
  *flags|=4;return 0;
 }while(count<5);
 return 1;
}

INCLUDE_ASM("asm/game", func_002DE7A0);

INCLUDE_ASM("asm/game", func_002DE7F0);

void func_002DE810(unsigned short *a0,u8 *a1) {
 a0[0]=0;
 a0[1]=*(unsigned short *)(a1+0x24);
 a0[2]=0;
 a0[3]=*(unsigned short *)(a1+0x20);
 a0[4]=*(s32 *)(a1+0x20)>>1;
 a0[5]=*(s32 *)(a1+0x24)>>1;
 a0[8]=0x10;a0[9]=0;
}

INCLUDE_ASM("asm/game", func_002DE850);

INCLUDE_ASM("asm/game", func_002DE858);

INCLUDE_ASM("asm/game", func_002DE900);

INCLUDE_ASM("asm/game", func_002DE990);

INCLUDE_ASM("asm/game", func_002DF050);

INCLUDE_ASM("asm/game", func_002DF158);

INCLUDE_ASM("asm/game", func_002DF190);
