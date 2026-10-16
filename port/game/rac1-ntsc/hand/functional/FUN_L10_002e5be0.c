/* Carried from the PAL decompilation's nonmatching/l10_orxon/func_L10_002E6FA0.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L10_002e5be0 -- src/overlays/l10_orxon/vendor_002E30F8.c
 * Best so far: SIZE ours 736 / retail 732, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   mini16 a03: emits randomized facing-direction particles. Best p7.c BYTES52/732: prologue/loop-entry setup (+4.
 *   Explicit source position, guarded loop, shared scale call after selecting bounds, and actual pointer-return sp
 */
extern int func_00213260(int);
extern float func_001FA580(float, float);
extern float func_001F9DC8(float);
extern float func_002132A8(float, float);
extern float func_001F9DE0(float);
extern void func_001F9A10(void *, void *, void *);
extern void func_00214DB0(void *, float, float, float);
extern void func_001F9A68(void *, void *, float);
extern int func_001FA6E0(int, int, float);
extern float func_001F96B0(float);
extern int func_001FA898_r(float) __asm__("func_001FA6D0");
extern void func_L00_002738E8(void *, void *, int, float, float, float, int, int, int);
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED70 MACRO_ADDR;
extern int D_L10_00161F04 SDATA(D_L10_00161F04);
extern float D_L10_00161EFC SDATA(D_L10_00161EFC);
extern float D_L10_00161EF8 SDATA(D_L10_00161EF8);
extern int D_L10_00161EF0 SDATA(D_L10_00161EF0);
extern int D_L10_00161EF4 SDATA(D_L10_00161EF4);
extern float D_L10_00161F00 SDATA(D_L10_00161F00);
/* Emits randomized particles in the object's facing direction. */
void FUN_L10_002e5be0(char *m) {
 float pos[4],vel[4],off[4];
 char *d=*(char **)(m+0x78);
 char *sourcepos;
 int i;
 ((unsigned char *)m)[0x30]=0x80;
 if(*(int *)(d+0xC)>0) {
 sourcepos=m+0x10;
 for(i=0;i<*(int *)(d+0xC);i++) {
  int small=func_00213260(100)<D_L10_00161F04;
  float a,b,c,scale;
  int col,fade,life;
  qcopy(pos,sourcepos);
  off[0]=func_001F9DC8(func_001FA580(*(float *)(m+0x48),1.5707964f))*func_002132A8(-*(float *)(d+0x10),*(float *)(d+0x10));
  off[1]=func_001F9DE0(func_001FA580(*(float *)(m+0x48),1.5707964f))*func_002132A8(-*(float *)(d+0x10),*(float *)(d+0x10));
  off[2]=0;
  off[2]=func_002132A8(-*(float *)(d+0x14),*(float *)(d+0x14));
  func_001F9A10(pos,pos,off);
  a=func_002132A8(D_L10_00161EFC,D_L10_00161EF8);
  b=func_002132A8(D_L10_00161EFC,D_L10_00161EF8);
  c=func_002132A8(D_L10_00161EFC,D_L10_00161EF8);
  if(a<b) a=b;
  if(a<c) a=c;
  func_00214DB0(vel,a*D_0015ED6C,*(float *)(m+0x48),-*(float *)(m+0x44));
  if(small) {
   func_001F9A68(vel,vel,func_002132A8(0.5f,1.5f));
   scale=func_002132A8(0.24f,0.36f);
  } else scale=func_002132A8(0.8f,1.2f);
  scale=*(float *)(d+4)*scale;
  col=func_001FA6E0(D_L10_00161EF0,D_L10_00161EF4,func_002132A8(0.5f,1.0f));
  fade=func_001FA6E0(D_L10_00161EF4,D_L10_00161EF4 & 0xFF000000,func_002132A8(0.25f,0.5f));
  life=func_001FA898_r(func_001F96B0(*(float *)(d+8)*60.0f));
  func_L00_002738E8(pos,vel,life,scale,*(float *)d,D_L10_00161F00*D_0015ED70,col,fade,small);
 }
 }
}
