/* NON_MATCHING func_L03_00251AC8 -- src/overlays/shared/mobyutil_00249488.c
 * Best so far: SIZE ours 704 / retail 716, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Emits two randomized particle rings about the moby axis. Stopped at three-distinct-wording tie: p2 for, p3 whi
 *   Retail retains the position stack address in another saved register and sets first-ring random-angle arguments
 *   Original allocation/scheduling behavior would unblock these differences; initial position uses a plain 128-bit
 */
extern float func_L00_00258C80(float,float);
extern void func_002156E0(void *,void *,void *,float);
extern float func_002140F8(float,float);
extern void func_L00_001FF4B0(void *,void *,float);
extern int func_L00_00258BC8(int,int);
extern int func_002140B0(int);
extern int func_001F9850(int);
extern void func_L00_001FF240(void *,void *,void *);
extern unsigned char *func_L00_00272158_order(float,float,void *,float *,int,int,int,int,float,float,float) __asm__("func_L00_00272158");
extern int func_001FA898_r(float) __asm__("func_001FA898");
extern void func_L00_0026DA50(void *,void *,int,int,int,int,float);
typedef int U128 __attribute__((mode(TI)));
/* Emits two rings of randomly varied particles around the moby's axis. */
void func_L03_00251AC8(char *moby,float *position,void *extra) {
 float pos[4],vec[4],tmp[4];int i,j;
 *(U128 *)pos=*(U128 *)position;
 for(i=0;i<func_001FA898_r(24.0f);i++) {
  int n,sign,a,b;
  float angle=func_L00_00258C80(0.0f,0.1308996975f);
  func_002156E0(vec,moby+0xE0,moby+0xC0,(float)i*0.261799395f+angle);
  func_L00_001FF4B0(vec,vec,func_002140F8(0.05f,0.1f));
  sign=func_L00_00258BC8(0,3);if(func_002140B0(2))sign=-sign;
  a=func_001F9850(30);b=func_001F9850(90);n=func_L00_00258BC8(a,b);
  if(extra)func_L00_001FF240(tmp,vec,extra);
  func_L00_00272158_order(60000.0f,3000.0f,pos,vec,n,30,0xFFFFFF,sign,0.85f,-0.001f,0.85f);
 }
 for(j=0;j<func_001FA898_r(6.0f);j++) {
  int n,a,b;
  float angle=func_002140F8(0.0f,0.785398185f);
  func_002156E0(vec,moby+0xE0,moby+0xC0,(float)j*1.04719758f+angle);
  func_L00_001FF4B0(vec,vec,func_002140F8(0.05f,0.1f));
  a=func_001F9850(20);b=func_001F9850(60);n=func_L00_00258BC8(a,b);
  if(extra)func_L00_001FF240(tmp,vec,extra);
  func_L00_0026DA50(pos,vec,0x4F007FFF,0x1FFFFFFF,n,1,10000.0f);
 }
}
