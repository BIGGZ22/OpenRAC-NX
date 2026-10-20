#include "include_asm.h"

typedef void *V;

extern int D_001A8FF0;

extern void func_00282C98(V);

extern void func_002AFFD8(V,V,int);

void func_L00_003252C8(V p,V q) {
 if(D_001A8FF0==0) {
  func_00282C98(q);
  return;
 }
 func_002AFFD8(p,q,1);
}
