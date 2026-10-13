/* Carried from port/game/rac1-pal/hand/level_hero_heading.c through the version map (../README.md): names and
 * addresses are this version's, raw addresses in comments are PAL's. Not yet checked in play. */
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* SPDX-License-Identifier: GPL-3.0-or-later */
/* Native PAL hero heading/velocity adjustment (00214520), recovered from
 * nonmatching/shared/FUN_L00_00213e68.c and the full 0x2A0 retail body.
 * Not a PS2 match. Scratch w is initialized because the vector helpers
 * copy it; it has no effect on the result, whose w comes from velocity. */
typedef struct { float v[4]; } HeroHeadingVector;
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
 HeroHeadingVector projection;
 char pad930[5972];
 int state;
 char pad2088[4];
 int movement;
 char pad2090[35];
 unsigned char mode;
} HeroHeading;
extern HeroHeading D_0013F350;
extern float D_0015ED6C MACRO_ADDR;
extern float func_001F9DC8(float);
extern float func_001F9DE0(float);
extern float func_001F9B20(void *);
extern void func_001F9A28(void *,void *,void *);
extern void func_L00_001FF550(float *,float *,float);
extern void func_001F9A10(void *,void *,void *);
extern float func_L00_00213350(HeroHeadingVector *);
extern float func_L00_002339D0(float *);
extern float func_00213ED8(float *,float,float);
extern void func_L00_00233BA0(float *,float *,float);
extern void func_L00_00233708(void *,void *,float);
/* Adjusts hero movement toward heading or damps it according to movement mode. */
void FUN_L00_00213e68(float amount) {
 float v[4] = {0, 0, 0, 0}; HeroHeadingVector copy; float damping;
 int mode=D_0013F350.mode;
 switch(mode) {
 case 0: {
  float angle=D_0013F350.angle;
  float speed=D_0013F350.speed;
  float dot,len,other,limit;
  if(D_0013F350.movement==4) {
   if(D_0013F350.airborne==0 && D_0013F350.modeFlag!=0 && D_0013F350.boost!=0)
    speed=D_0015ED6C*2.1f;
  }
  if(D_0013F350.airborne!=0 && D_0013F350.state==14) {
   if(D_0015ED6C<speed) speed=D_0015ED6C;
  }
  v[0]=func_001F9DC8(angle)*speed;
  v[1]=func_001F9DE0(angle)*speed;
  dot=-(D_0013F350.velocity[0]*v[0]+D_0013F350.velocity[1]*v[1]);
  v[2]=D_0013F350.velocity[2];
  len=func_001F9B20(v);
  other=func_001F9B20(D_0013F350.velocity);
  if(len==0.0f || other==0.0f) dot=0.0f;
  else {dot/=len;dot/=other;}
  speed=dot+1.0f;
  speed*=1.5f;
  limit=(D_0013F350.state==0x81)?1.7f:3.0f;
  if(limit<speed) speed=limit;
  if(speed<1.0f) speed=1.0f;
  { float *vel=D_0013F350.velocity;
  func_001F9A28(v,v,vel);
  amount*=speed;
  if(amount<func_001F9B20(v)) func_L00_001FF550(v,v,amount);
  func_001F9A10(vel,vel,v);
  for (int i = 0; i < 4; ++i) D_0013F350.previous[i] = vel[i];
  copy=D_0013F350.projection;
  dot=func_L00_00213350(&copy);
  D_0013F350.heightDelta=D_0013F350.height-dot;
  }
 break;
 }
 case 1:
 case 2: {
  float *vel=D_0013F350.velocity;
  damping=func_L00_002339D0(vel);
  func_00213ED8(&damping,D_0013F350.speed,amount);
  func_L00_00233BA0(vel,vel,0.0f);
  func_L00_00233708(vel,vel,damping);
 break;
 }
 }
}
