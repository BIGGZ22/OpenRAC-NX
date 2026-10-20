/* 0 of 7 places match - last line: 12_todano: 236 bytes, expected 228
   Both blocks, the two  float/int pairs, the two helper calls and the 0x5F/0x6F vs
   0x6F/0x7F constants are understood. This cc1 spends one more callee-saved register on
   the parameter copy (s0 = a0, s1 = s0+0x10) where retail keeps the parameter itself in
   s1 (s1 = a0, s2 = s1+0x10) and gives s0 to the first call's result; two instructions
   over. Needs a -G8 unit (the -g8 sibling exists). Declaring the result first and
   deriving s2 from s1 did not move the assignment. */

typedef unsigned char u8;

extern float D_L12_001B1560 __attribute__((sda));

extern float D_L12_001B1564 __attribute__((sda));

extern int D_L12_001B1558 __attribute__((sda));

extern int D_L12_001B1520 __attribute__((sda));

extern int D_L12_001B1524 __attribute__((sda));

extern int D_L12_001B155C __attribute__((sda));

extern int func_002A7820(int,int);

extern float func_002A7940(int);

extern void func_L12_00408368(void *,int,void *,int,int,void *,float,float,float);

void func_L12_004081F8(u8 *a0) {
 u8 *s1=a0;
 u8 *s2=s1+0x10;
 float f;
 void *p;
 f=D_L12_001B1560**(float *)(s1+0x2C);
 p=(void *)func_002A7820(0x5F,0x6F);
 func_L12_00408368(s2,D_L12_001B1520,p,D_L12_001B1558,2,s1,f,func_002A7940(0x5F),1.0f);
 f=D_L12_001B1560**(float *)(s1+0x2C)*D_L12_001B1564;
 p=(void *)func_002A7820(0x6F,0x7F);
 func_L12_00408368(s2,D_L12_001B1524,p,D_L12_001B155C,2,s1,f,func_002A7940(0x6F),1.0f);
}