/* NON_MATCHING func_L05_00254B38 -- src/overlays/shared/help_00237B00.c
 * Best so far: SIZE ours 680 / retail 688, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Finds the nearest eligible path edge behind the hero, returns its position/ground height and heading. Budget8 
 *   Remaining differences: hero-base lifetime/prologue, distance/height-limit float registers, wrapped-index addit
 *   Actual globals D_0013F450 and D_0013F4D0 replace malformed assembly roots; block-local allocation and arithmet
 */
extern float func_L00_001FF860(float, float);
extern float func_001FA850(float, float);
extern float func_001F9B88(float);
extern float func_001F9D48(void *, void *);
extern float func_00214358(void *, int, float);
extern void func_001F9BC0(void *);
extern char *D_L05_001B0CB0[];
extern short D_0015EE84;
extern unsigned char D_0013F450_path[] __asm__("D_0013F450");
extern float D_0013F4D0[] MACRO_ADDR;
/* Finds a nearby path edge behind the hero and returns its position and heading. */
void func_L05_00254B38(float *out,float *rotation) {
 char *g=(char *)D_0013F450_path;
 char *m=*(char **)(g+0x86C);
 char *path,*p,*point;int i,best=0;
 float distance,limit;
 float tmp[4];
 if(!m) {qcopy(out,g+0x80);qcopy(rotation,g+0x90);return;}
 path=D_L05_001B0CB0[*(int *)(*(char **)(m+0x78)+0x24)];
 distance=9999999.0f;i=0;
 if(*(int *)path>0) {
 limit=4.0f;p=path;point=path+0x10;
 do {
  float angle=func_L00_001FF860(*(float *)(p+0x10)-*(float *)(g+0x80),*(float *)(p+0x14)-*(float *)(g+0x84));
  int next=(i+*(int *)path+2)%*(int *)path;
  float edge=func_L00_001FF860(*(float *)(path+next*16+0x10)-*(float *)(p+0x10),*(float *)(path+next*16+0x14)-*(float *)(p+0x14));
  if(func_001FA850(angle,edge)>1.57079637f) {
   if(*(int *)&D_0015EE84==16 && *(char **)(g+0x8B4)) {
    char *verts=*(char **)(g+0x8B4);
    float z1=*(float *)(verts+*(short *)(g+0x8C8)*16+0x18);
    float z2=*(float *)(verts+*(short *)(g+0x898)*16+0x18);
    float dz;
    if(z1<z2)dz=z2-*(float *)(p+0x18);else dz=z1-*(float *)(p+0x18);
    if(func_001F9B88(dz)>limit)goto nextpoint;
   }
   {
    float d=func_001F9D48(D_0013F4D0,point);
    if(d<distance) {distance=d;best=i;}
   }
  }
nextpoint:
 i+=2;p+=0x20;point+=0x20;
 }while(i<*(int *)path);
 }
 qcopy(out,path+best*16+0x10);
 qcopy(tmp,out);tmp[2]+=2.0f;
 out[2]=func_00214358(tmp,0,0.5f);
 func_001F9BC0(rotation);
 {float *a=(float *)(path+best*16);float *b=(float *)(path+(best+1)*16);
 rotation[2]=func_L00_001FF860(b[4]-a[4],b[5]-a[5]);}
}
