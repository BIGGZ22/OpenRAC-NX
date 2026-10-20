/* Alternative source form for func_L12_004081F8.
   Retained from 4081F8_a.c, entry func_L12_004081F8; declarations preserved.
   Naming/extraction only: this form has not been recompiled or validated
   by the cleanup. See the primary near-miss file for the recorded blocker;
   do not apply another form's verdict to this one. Recheck its file-level
   flags with tools/check.py, then tools/build.py before promotion. */
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
 u8 *s2=a0+0x10;
 float f;
 void *p;
 f=D_L12_001B1560**(float *)(s1+0x2C);
 p=(void *)func_002A7820(0x5F,0x6F);
 func_L12_00408368(s2,D_L12_001B1520,p,D_L12_001B1558,2,s1,f,func_002A7940(0x5F),1.0f);
 f=D_L12_001B1560**(float *)(s1+0x2C)*D_L12_001B1564;
 p=(void *)func_002A7820(0x6F,0x7F);
 func_L12_00408368(s2,D_L12_001B1524,p,D_L12_001B155C,2,s1,f,func_002A7940(0x6F),1.0f);
}
