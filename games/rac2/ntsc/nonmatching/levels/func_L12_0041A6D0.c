/* 0 of 2 places match - last line: 12_todano: 88 bytes, expected 92
   The three guards (block[0x68] == 0, block[0x24] == -1, unsigned a1 < 10), the two
   stores and the +0x7D0 stamp are understood; retail keeps the parameter in $a3 and
   loads the table base out of it (lw $a0,0x68($a3)) while this cc1 uses the parameter
   register directly, so the failure blocks land elsewhere and the body is one store
   shorter. guard-chain and &&-with-single-return forms both fail the same way. */

typedef unsigned char u8;

extern u8 D_001A63A8[];

int func_L12_0041A6D0(u8 *a0,int a1) {
 u8 *b=D_001A63A8;
 if(*(int *)(b+0x68)!=0) return 0;
 if(*(int *)(b+0x24)!=-1) return 0;
 if((unsigned)a1>=10) return 0;
 *(int *)(b+0x28)=(int)a0;
 *(int *)(b+0x24)=*(int *)(*(int *)(a0+0x68)+a1*4+0x230)+0x7D0;
 return 1;
}