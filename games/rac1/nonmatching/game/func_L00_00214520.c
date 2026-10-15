/* NON_MATCHING func_L00_00214520 -- src/overlays/shared/help_0020CDF0.c
 * Best so far: SIZE ours 668 / retail 672, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   mini12/a03 budget 8 spent. Adjusts hero movement heading and speed or damps velocity. Best p6/p7: 668/672; all
 *   Remaining clamp mul.s f20,f20,f0 moves from offset180 into the state-compare delay slot18c; retail has nop the
 *   Typed hero fields (actual D0013F450), switch modes, late actual velocity D0013F530 pointer, and owning-hero de
 */
#include "common.h"
typedef int Help14520Quad __attribute__((mode(TI)));
typedef struct {
 char pad0[224];
 float velocity[4];
 char padf0[64];
 float previous[4];
 char pad140[40];
 float height;
 char pad16c[20];
 float angle;
 char pad184[12];
 float speed;
 float heightDelta;
 char pad198[191];
 unsigned char boost;
 char pad258[178];
 short airborne;
 char pad30c[408];
 short modeFlag;
 char pad4a6[1146];
 QVec projection;
 char pad930[5972];
 int state;
 char pad2088[4];
 int movement;
 char pad2090[35];
 unsigned char mode;
} Hero14520;
extern Hero14520 hero14520 __asm__("D_0013F450");
extern unsigned char D_0013F530[];
extern float D_0015EE6C MACRO_ADDR;
extern short D_0015EE6C_gp __asm__("D_0015EE6C");
extern float func_001F9F90(float);
extern float func_001F9FA8(float);
extern float func_001F9CE8(void *);
extern void func_001F9BF0_113A0(void *,void *,void *) __asm__("func_001F9BF0");
extern void func_L00_001FF500(float *,float *,float);
extern void func_001F9BD8(void *,void *,void *);
extern float func_L00_00213A08(QVec *);
extern float func_L00_00234250(float *);
extern float func_00214D28(float *,float,float);
extern void func_L00_00234420(float *,float *,float);
extern void func_L00_00233F88(void *,void *,float);
/* Adjusts hero movement toward heading or damps it according to movement mode. */
void func_L00_00214520(float amount) {
 float v[4]; QVec copy; float damping;
 int mode=hero14520.mode;
 switch(mode) {
 case 0: {
  float angle=hero14520.angle;
  float speed=hero14520.speed;
  float dot,len,other,limit;
  if(hero14520.movement==4) {
   if(hero14520.airborne==0 && hero14520.modeFlag!=0 && hero14520.boost!=0)
    speed=*(float *)&D_0015EE6C_gp*2.1f;
  }
  if(hero14520.airborne!=0 && hero14520.state==14) {
   if(D_0015EE6C<speed) speed=D_0015EE6C;
  }
  v[0]=func_001F9F90(angle)*speed;
  v[1]=func_001F9FA8(angle)*speed;
  dot=-(hero14520.velocity[0]*v[0]+hero14520.velocity[1]*v[1]);
  v[2]=hero14520.velocity[2];
  len=func_001F9CE8(v);
  other=func_001F9CE8(hero14520.velocity);
  if(len==0.0f || other==0.0f) dot=0.0f;
  else {dot/=len;dot/=other;}
  speed=dot+1.0f;
  speed*=1.5f;
  limit=(hero14520.state==0x81)?1.7f:3.0f;
  if(limit<speed) speed=limit;
  if(speed<1.0f) speed=1.0f;
  { float *vel=(float *)D_0013F530;
  func_001F9BF0_113A0(v,v,vel);
  amount*=speed;
  if(amount<func_001F9CE8(v)) func_L00_001FF500(v,v,amount);
  func_001F9BD8(vel,vel,v);
  qcopy(hero14520.previous,vel);
  copy=hero14520.projection;
  dot=func_L00_00213A08(&copy);
  { Hero14520 *h=(Hero14520 *)((char *)vel-0xE0);
    h->heightDelta=h->height-dot; }
  }
 break;
 }
 case 1:
 case 2: {
  float *vel=hero14520.velocity;
  damping=func_L00_00234250(vel);
  func_00214D28(&damping,hero14520.speed,amount);
  func_L00_00234420(vel,vel,0.0f);
  func_L00_00233F88(vel,vel,damping);
 break;
 }
 }
}
