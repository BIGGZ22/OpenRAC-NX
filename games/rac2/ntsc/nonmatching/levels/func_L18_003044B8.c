/* 0 of 3 places match - last line: 18_damosel: 152 bytes, expected 192
   float -> int conversion of the third parameter, a half-height shift, two helper calls.
   Retail keeps SIX callee-saved slots (s0..s3 + f21 + f22) and caches each int parameter
   in its own, and moves the incoming f12 into f22 before the first call; this cc1 needs
   far fewer and orders the saves differently. 40 bytes short; the argument roles
   (a3/a4 to the first call, a2/a3/a4/f/n to the second) are confident. */

typedef unsigned char u8;

extern int func_0027C878(int);

extern void func_L18_00303A58(void *,void *);

extern void func_0027F640(void *,void *,int,int,void *);

void func_L18_003044B8(int a0,int a1,void *a2,int a3,int a4,float f) {
 int n;
 n=(int)f;
 func_L18_00303A58(a3,a4);
 n=a0-(n>>1);
 func_0027C878(3);
 func_0027F640(a2,a3,a4,f,n);
}