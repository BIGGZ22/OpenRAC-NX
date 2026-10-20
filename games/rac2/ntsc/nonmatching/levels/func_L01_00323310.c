/* 0 of 18 places match - last line: 01_oozla: 104 bytes, expected 112
   The state test, the two movn masks (value != -1 -> & 0xFF) and the three shorts are
   understood, but retail keeps the key in $a2 (a copy of $a0) so $a0 can hold ~a1,
   materialises the block late (lui $t0 + addiu $v0) and carries the byte length in
   $t1; this cc1 copies a1 instead. Eight bytes short. */

typedef unsigned char u8;

extern u8 D_001A63A8[];

void func_L01_00323310(int a0,int a1) {
 u8 *b=D_001A63A8;
 int v;
 if(*(short *)(b+0x48)==a0&&*(short *)(b+0x3C)==-1) return;
 v=(a0!=-1)?(a0&0xFF):a0;
 v=(a1!=-1)?(v&0xFF):v;
 if(*(short *)(b+0x4E)!=0||*(short *)(b+0x96)!=0) {
  *(short *)(b+0x3E)=a1;
  *(short *)(b+0x3C)=v;
  return;
 }
 *(short *)(b+0x48)=v;
}