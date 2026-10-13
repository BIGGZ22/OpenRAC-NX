/* Carried from the PAL decompilation's nonmatching/shared/func_L03_00251AC8.c (games/rac1/pal) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* NON_MATCHING FUN_L03_00250ae8 -- src/overlays/shared/mobyutil_00249488.c
 * Best so far: SIZE ours 704 / retail 716, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Emits two randomized particle rings about the moby axis. Stopped at three-distinct-wording tie: p2 for, p3 whi
 *   Retail retains the position stack address in another saved register and sets first-ring random-angle arguments
 *   Original allocation/scheduling behavior would unblock these differences; initial position uses a plain 128-bit
 */
extern float func_L00_00257C48(float,float);
extern void func_00214890(void *,void *,void *,float);
extern float func_002132A8(float,float);
extern void func_L00_001FF500(void *,void *,float);
extern int func_L00_00257B90(int,int);
extern int func_00213260(int);
extern int func_001F9688(int);
extern void func_L00_001FF290(void *,void *,void *);
extern unsigned char *func_L00_00272158_order(float,float,void *,float *,int,int,int,int,float,float,float) __asm__("func_L00_002712B8");
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_L00_0026CBB0(void *,void *,int,int,int,int,float);
typedef int U128_c250ae8 __attribute__((mode(TI)));
/* Emits two rings of randomly varied particles around the moby's axis. */
void FUN_L03_00250ae8(char *moby,float *position,void *extra) {
 float pos[4],vec[4],tmp[4];int i,j;
 *(U128_c250ae8 *)pos=*(U128_c250ae8 *)position;
 for(i=0;i<func_001FA898_r(24.0f);i++) {
  int n,sign,a,b;
  float angle=func_L00_00257C48(0.0f,0.1308996975f);
  func_00214890(vec,moby+0xE0,moby+0xC0,(float)i*0.261799395f+angle);
  func_L00_001FF500(vec,vec,func_002132A8(0.05f,0.1f));
  sign=func_L00_00257B90(0,3);if(func_00213260(2))sign=-sign;
  a=func_001F9688(30);b=func_001F9688(90);n=func_L00_00257B90(a,b);
  if(extra)func_L00_001FF290(tmp,vec,extra);
  func_L00_00272158_order(60000.0f,3000.0f,pos,vec,n,30,0xFFFFFF,sign,0.85f,-0.001f,0.85f);
 }
 for(j=0;j<func_001FA898_r(6.0f);j++) {
  int n,a,b;
  float angle=func_002132A8(0.0f,0.785398185f);
  func_00214890(vec,moby+0xE0,moby+0xC0,(float)j*1.04719758f+angle);
  func_L00_001FF500(vec,vec,func_002132A8(0.05f,0.1f));
  a=func_001F9688(20);b=func_001F9688(60);n=func_L00_00257B90(a,b);
  if(extra)func_L00_001FF290(tmp,vec,extra);
  func_L00_0026CBB0(pos,vec,0x4F007FFF,0x1FFFFFFF,n,1,10000.0f);
 }
}
