/* Carried from the PAL decompilation's nonmatching/shared/func_L05_00254B38.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L05_00254358 -- src/overlays/shared/help_00237B00.c
 * Best so far: SIZE ours 680 / retail 688, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Finds the nearest eligible path edge behind the hero, returns its position/ground height and heading. Budget8 
 *   Remaining differences: hero-base lifetime/prologue, distance/height-limit float registers, wrapped-index addit
 *   Actual globals D_0013F350 and D_0013F3D0 replace malformed assembly roots; block-local allocation and arithmet
 */
extern float func_L00_001FF8B0(float, float);
extern float func_001FA688(float, float);
extern float func_001F99C0(float);
extern float func_001F9B80(void *, void *);
extern float func_00213508(void *, int, float);
extern void func_001F99F8(void *);
extern char *D_L05_001B0930[];
extern short D_0015ED84;
extern unsigned char D_0013F450_path[] __asm__("D_0013F350");
extern float D_0013F3D0[] MACRO_ADDR;
/* Finds a nearby path edge behind the hero and returns its position and heading. */
void FUN_L05_00254358(float *out,float *rotation) {
 char *g=(char *)D_0013F450_path;
 char *m=*(char **)(g+0x86C);
 char *path,*p,*point;int i,best=0;
 float distance,limit;
 float tmp[4];
 if(!m) {qcopy(out,g+0x80);qcopy(rotation,g+0x90);return;}
 path=D_L05_001B0930[*(int *)(*(char **)(m+0x78)+0x24)];
 distance=9999999.0f;i=0;
 if(*(int *)path>0) {
 limit=4.0f;p=path;point=path+0x10;
 do {
  float angle=func_L00_001FF8B0(*(float *)(p+0x10)-*(float *)(g+0x80),*(float *)(p+0x14)-*(float *)(g+0x84));
  int next=(i+*(int *)path+2)%*(int *)path;
  float edge=func_L00_001FF8B0(*(float *)(path+next*16+0x10)-*(float *)(p+0x10),*(float *)(path+next*16+0x14)-*(float *)(p+0x14));
  if(func_001FA688(angle,edge)>1.57079637f) {
   if(*(int *)&D_0015ED84==16 && *(char **)(g+0x8B4)) {
    char *verts=*(char **)(g+0x8B4);
    float z1=*(float *)(verts+*(short *)(g+0x8C8)*16+0x18);
    float z2=*(float *)(verts+*(short *)(g+0x898)*16+0x18);
    float dz;
    if(z1<z2)dz=z2-*(float *)(p+0x18);else dz=z1-*(float *)(p+0x18);
    if(func_001F99C0(dz)>limit)goto nextpoint;
   }
   {
    float d=func_001F9B80(D_0013F3D0,point);
    if(d<distance) {distance=d;best=i;}
   }
  }
nextpoint:
 i+=2;p+=0x20;point+=0x20;
 }while(i<*(int *)path);
 }
 qcopy(out,path+best*16+0x10);
 qcopy(tmp,out);tmp[2]+=2.0f;
 out[2]=func_00213508(tmp,0,0.5f);
 func_001F99F8(rotation);
 {float *a=(float *)(path+best*16);float *b=(float *)(path+(best+1)*16);
 rotation[2]=func_L00_001FF8B0(b[4]-a[4],b[5]-a[5]);}
}
