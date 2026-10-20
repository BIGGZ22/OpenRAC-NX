/* 0 of 12 places match - last line: 18_damosel: %hi at +0x0 differs
   Right size (48) and the whole flow (short-circuit ||, the result built in $a1 with one
   shared jr) is understood; this cc1 only swaps the address register: it materialises the
   first global as lui $v1 / lw $v0,0($v1) where retail keeps base and value in $v0
   (lui $v0 / lw $v0,off($v0)). Early-return and result-variable forms agree. */

typedef unsigned char u8;

extern int D_001AFBB0;

extern u8 D_0018C0D4[];

int func_L18_002F1098(void) {
 int r=0;
 if(D_001AFBB0!=0||D_0018C0D4[0]==4) r=1;
 return r;
}