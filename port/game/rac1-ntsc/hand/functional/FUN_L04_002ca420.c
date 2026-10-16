/* Carried from the PAL decompilation's src/overlays/l04_eudora/vendor_002CB800.c (matched C there) (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's.
 * The two versions' code differs here: func_L04_002CB800 (near) was paired by place; read it against NTSC-U before trusting it. */
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
#define MOBYS ((VehicleMoby_2CB800_c2ca420 *)D_L04_0015FFD8)
extern char *D_L04_0015FFD8 MACRO_ADDR;
extern void func_0022DA68(int, int, int);
typedef int VehicleWide_2CB800_c2ca420 __attribute__((mode(TI)));
typedef union { VehicleWide_2CB800_c2ca420 quad; float f[4]; } VehicleVec_2CB800_c2ca420;
typedef union { s32 id; u8 token; } VehicleTrigger_2CB800_c2ca420;
typedef struct {
    char pad0[0x60];
    char motion[0x14]; s32 path_id;
    char pad78[4]; float turn_velocity;
    char pad80[0x7C]; float speed;
    char pad100[0x1C]; s32 moving_timer;
    char pad120[0x18]; float start_speed;
    char pad13C[4]; float angle0,angle1;
    u8 selected, phase; s16 phase_timer;
    float start_angle; VehicleTrigger_2CB800_c2ca420 paths[4];
    VehicleVec_2CB800_c2ca420 start_pos;
    s32 visibility_timer, visibility_active, shield, partner;
    char matrix0[0x40],matrix1[0x40];
} VehicleData_2CB800_c2ca420;
typedef struct {
    char pad0[0x10]; VehicleVec_2CB800_c2ca420 pos;
    u8 state; char pad21[3]; char *metadata;
    char pad28[8]; u8 alpha; char pad31; s16 shield; u16 flags;
    char pad36[0xA]; VehicleVec_2CB800_c2ca420 rot;
    char pad50[0x28]; VehicleData_2CB800_c2ca420 *data;
    char pad7C[0x18]; s32 sort;
    char pad98[0xC]; u8 draw_alpha; char padA5; s16 cls;
    char padA8[0x18]; char matrix[0x40];
} VehicleMoby_2CB800_c2ca420;
typedef struct { char pad[0x30]; VehicleVec_2CB800_c2ca420 pos; char pad40[0x38]; float angle; char pad7C[4]; } VehiclePath_2CB800_c2ca420;
typedef struct { VehicleVec_2CB800_c2ca420 vec; VehicleMoby_2CB800_c2ca420 *owner; s32 flags, unused; float strength; s32 result; } VehicleQuery_2CB800_c2ca420;
extern s32 *D_L04_001B0630[];
extern VehiclePath_2CB800_c2ca420 *D_L04_001600EC MACRO_ADDR;
extern char D_L04_001DBE10[];
extern s32 D_0015EDF8 MACRO_ADDR;
typedef struct { char pad[6]; u8 flag; } VehicleWeapon_2CB800_c2ca420;
extern VehicleWeapon_2CB800_c2ca420 D_0013D408;
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
extern float D_L04_0015F580[] MACRO_ADDR;
typedef struct { char pad[0x140]; float x,y,z; char pad14C[12]; float angle; } VehicleCamera_2CB800_c2ca420;
extern VehicleCamera_2CB800_c2ca420 D_L04_00166E00;
extern float func_001F9B48(void *,void *);
extern s32 func_001FA6D0(float);
extern s32 func_001E93B0();
extern void func_L01_002F5168(VehicleMoby_2CB800_c2ca420 *);
extern void func_001F9770(void *);
extern s32 func_00214720(void *,s32);
extern void func_L00_00258278(void *,void *,float,float,float,float);
extern float func_001F9B80(void *,void *);
extern float func_001FA688(float,float);
extern s32 func_001F9688(s32);
extern void func_001F9A28(void *,void *,void *);
extern void func_L00_001FF500(void *,void *,float);
extern void func_001F9A10(void *,void *,void *);
extern float func_001F9AF0(void *);
extern void func_0020CB10(void *,s32,void *);
extern float func_001FA580(float,float);
extern void func_L00_001FFF28(void *,s32,float);
extern void func_001F9CF8(void *,void *,void *);
extern void func_L00_00250480(void *);
extern void func_L00_00250DF8(void *);
extern void func_0020E098(void *);
extern char *func_L00_0025A420(void *,s32,s32);
extern s32 func_0022DB10(s32,s32,s32);
extern void func_L00_00263D40(s32,s32);
extern void func_001F99F8(void *);
extern void func_L00_001F2868(void *,float,s32,void *,void *);
extern void func_L00_0025E450(void *,void *,void *,float,float,s32,s32,s32,float,float,float,float,s32,float,s32,s32,s32,s32);
extern void func_L00_0024F7C8(void *,s32,void *);
extern void func_L00_0025F090(void *,void *,s32,float,float);
extern void func_001F9A68(void *,void *,float);
extern void func_L00_00263FD8(void *,s32,void *,void *,s32,s32,float,void *,void *,void *);
extern s32 func_001F9740(s32 *);
extern float func_L00_001FF8B0(float,float);
typedef float VehicleArr_2CB800_c2ca420[4] __attribute__((aligned(16)));
typedef struct { char pad[0x14]; s32 path_id; } VehicleSub_2CB800_c2ca420;



/* Moves generic vehicle mobys along triggered paths and updates their attached parts, damage effects and visibility. */
void FUN_L04_002ca420(VehicleMoby_2CB800_c2ca420 *m) {
    VehicleVec_2CB800_c2ca420 old, delta;
    VehicleData_2CB800_c2ca420 *d=m->data;
    u8 state=m->state;
    VehicleSub_2CB800_c2ca420 *sub;
    VehicleTrigger_2CB800_c2ca420 *trigger;
    float zero; s32 izero;
    qcopy(&old,&m->pos);
    if (!d) return;
    sub=(VehicleSub_2CB800_c2ca420 *)d->motion;
    if (state==0) {
        if (sub->path_id!=-1) {
            s32 *path=D_L04_001B0630[sub->path_id];
            if (func_001F9B48((char *)path+0x10,(char *)path+(*path<<4))<0.5f) --*path;
        } else {
            func_001E93B0(D_L04_001DBE10,m->cls,func_001FA6D0((float)((s32)((char *)m-(char *)MOBYS)>>8)));
        }
        m->shield=0x7F;
        m->alpha=0xFF;
        d->start_speed=d->speed;
        d->shield=m->shield;
    }
    izero=0; zero=*(float *)&izero;
    func_L01_002F5168(m);
    if (m->cls!=0x1E4) {
        func_001F9770(&d->phase_timer);
        switch (d->phase) {
        case 0: {
            s32 i;
            if (d->speed<d->start_speed) {
                d->speed+=D_0015ED70*20.0f;
                if (d->speed>d->start_speed) d->speed=d->start_speed;
            }
            for (i=0,trigger=d->paths;i<4;i++,trigger++) {
                if (d->phase_timer==0 && d->paths[i].id!=-1 && func_00214720(&m->pos,d->paths[i].id)) {
                    qcopy(&d->start_pos,&m->pos);
                    d->selected=trigger->token;
                    d->phase=1;
                    d->start_angle=m->rot.f[2];
                    d->turn_velocity=0;
                    m->state=3;
                    d->moving_timer=func_001F9688(0x8CA0);
                    break;
                }
            }
            break;
        }
        case 1:
            func_L00_00258278(m,&d->turn_velocity,D_L04_001600EC[d->selected].angle,D_0015ED70*1.0471976f,D_0015ED70*3.1415927f,D_0015ED6C*3.1415927f);
            if (func_001F9B80(&m->pos,&D_L04_001600EC[d->selected].pos)<1.0f &&
                func_001FA688(m->rot.f[2],D_L04_001600EC[d->selected].angle)<0.17453292f) {
                d->phase=2;
                d->phase_timer=func_001F9688(300);
            }
            func_001F9A28(&delta,&D_L04_001600EC[d->selected].pos,&m->pos);
            func_L00_001FF500(&delta,&delta,D_0015ED6C*3.0f);
            func_001F9A10(&m->pos,&m->pos,&delta);
            break;
        case 2:
            if (d->phase_timer==0) { d->turn_velocity=zero; d->phase=3; }
            break;
        case 3: {
            float length,step;
            func_L00_00258278(m,&d->turn_velocity,d->start_angle,D_0015ED70*1.0471976f,D_0015ED70*3.1415927f,D_0015ED6C*3.1415927f);
            if (func_001F9B80(&m->pos,&d->start_pos)<1.0f && func_001FA688(m->rot.f[2],d->start_angle)<0.034906585f) {
                d->phase=0;
                d->phase_timer=func_001F9688(600);
                d->speed=zero;
                d->turn_velocity=zero;
                d->moving_timer=0;
            }
            func_001F9A28(&delta,&d->start_pos,&m->pos);
            length=func_001F9AF0(&delta);
            func_L00_001FF500(&delta,&delta,D_0015ED6C*3.0f>length ? length : D_0015ED6C*3.0f);
            func_001F9A10(&m->pos,&m->pos,&delta);
            break;
        }
        }
    }
    if (state==0) {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:case 19:case 20:case 28:case 29:case 32:case 89:
            func_0020CB10(m,0,d->matrix0);d->angle0=0;break;
        case 22:case 27:
            func_0020CB10(m,0,d->matrix0);d->angle0=0;
            func_0020CB10(m,1,d->matrix1);d->angle1=0;break;
        }
    } else {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:
            d->angle0=func_001FA580(d->angle0,D_0015ED6C*12.566371f);
            func_L00_001FFF28(d->matrix0+0x10,0,d->angle0);break;
        case 19:case 20:case 28:case 29:case 32:case 89:
            d->angle0=func_001FA580(d->angle0,D_0015ED6C*12.566371f);
            func_L00_001FFF28(d->matrix0+0x10,2,d->angle0);break;
        case 22:case 27:
            d->angle0=func_001FA580(d->angle0,D_0015ED6C*12.566371f);
            func_L00_001FFF28(d->matrix0+0x10,2,d->angle0);
            d->angle1=func_001FA580(d->angle1,D_0015ED6C*12.566371f);
            func_L00_001FFF28(d->matrix1+0x10,2,d->angle1);break;
        }
    }
    if (d->partner!=-1) {
        VehicleMoby_2CB800_c2ca420 *other=MOBYS+d->partner;
        other->shield=0x7F;
        switch (m->cls) {
        case 0x1E0:
            delta.quad=0;
            delta.f[0]=0.5f;delta.f[2]=-5.0f;
            func_001F9CF8(&delta,&delta,m->matrix);
            func_001F9A10(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            other->rot.f[1]=func_001FA580(other->rot.f[1],1.5707964f);
            func_L00_00250480(other);func_L00_00250DF8(other);func_0020E098(other);
            other->flags|=6;
            break;
        case 0x1EE:
            delta.quad=0;delta.f[2]=-2.0f;
            func_001F9CF8(&delta,&delta,m->matrix);
            func_001F9A10(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            func_L00_00250480(other);func_L00_00250DF8(other);func_0020E098(other);
            other->flags|=6;
            break;
        }
    }
    if (func_L00_0025A420(m,0x210000,0)) {
        if (++D_0015EDF8>=10 && D_0013D408.flag==0) {
            D_0013D408.flag=1;
            func_0022DB10(1,0,0);func_L00_00263D40(0x53DB,-1);
        }
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 19:case 20: {
            {
                VehicleQuery_2CB800_c2ca420 query;
                query.flags=0x810000;query.strength=1;query.owner=m;
                func_001F99F8(&query.vec);query.result=0;
                func_L00_001F2868(&m->pos,4.0f,16,m,&query);
            }
        }
        case 0:case 14:case 22:case 24:case 28:case 29:case 32:case 89: {
            {
                VehicleArr_2CB800_c2ca420 none;
                delta.quad=0;delta.f[2]=0.75f;
                func_001F99F8(none);
                func_001F9A10(&delta,&delta,&m->pos);
                func_L00_0025E450(m,none,&delta,0.0f,0.0f,10,3,4,2,1,100000,3,-1,15,1,1,-1,0);
                if (m->cls==0x1F2 || m->cls==0x22B) func_0022DA68(0,0,(s32)m);
                else if (m->cls!=0x1E6) func_0022DA68(1,0,(s32)m);
                d->visibility_timer=func_001F9688(600);d->visibility_active=1;
            }
            break;
        }
        case 27: {
            {
                VehicleArr_2CB800_c2ca420 offset={0.0f,0.0f,0.75f};
                VehicleArr_2CB800_c2ca420 none;
                VehicleQuery_2CB800_c2ca420 query2;
                func_001F99F8(none);
                func_L00_0024F7C8(m,0,&delta);
                func_001F9A10(&delta,&delta,offset);
                func_L00_0025F090(m,&delta,-1,1.0f,13.0f);
                func_L00_0024F7C8(m,1,&delta);
                func_001F9A10(&delta,&delta,offset);
                func_L00_0025F090(m,&delta,-1,1.0f,13.0f);
                func_001F9A10(&delta,offset,&m->pos);
                func_L00_0025E450(m,none,&delta,0.0f,0.0f,20,5,4,2,1,100000,3,-1,15,1,1,-1,0);
                func_0022DA68(1,0,(s32)m);
                query2.flags=0x810000;query2.strength=2;query2.owner=m;
                func_001F99F8(&query2.vec);query2.result=0;
                func_L00_001F2868(&m->pos,6.0f,16,m,&query2);
                d->visibility_timer=func_001F9688(600);d->visibility_active=1;
            }
            break;
        }
        }
        func_001F9A68(&delta,m->matrix,D_0015ED60*0.075f);
        delta.f[2]+=D_0015ED60*0.08f;
        if (m->cls==0x1D2) {
            func_L00_00263FD8(m,0x63F,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x641,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x642,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1E0) {
            func_L00_00263FD8(m,0x642,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x643,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x644,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1E6) {
            s32 i;
            func_L00_00263FD8(m,0x645,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            for (i=0;i<6;i++) {
            func_L00_00263FD8(m,0x646,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            }
        }
        else if (m->cls==0x1EA) {
            func_L00_00263FD8(m,0x647,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x648,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x649,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1EE) {
            func_L00_00263FD8(m,0x64D,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x64E,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1EF) {
            func_L00_00263FD8(m,0x64F,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x650,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x651,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x658,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1F2) {
            func_L00_00263FD8(m,0x652,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x653,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x654,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
        }
        else if (m->cls==0x1ED) {
            s32 j,k;
            func_L00_00263FD8(m,0x793,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            func_L00_00263FD8(m,0x794,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            for (j=0;j<3;j++) {
            func_L00_00263FD8(m,0x795,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            }
            for (k=0;k<3;k++) {
            func_L00_00263FD8(m,0x796,&m->pos,&m->rot,func_001F9688(90),0,D_0015ED70*12.0f,&delta,D_L04_0015F580,D_L04_0015F580);
            }
        }
    }
    m->draw_alpha=0xFF;
    if (d->visibility_active) {
        if (func_001F9740(&d->visibility_timer) &&
            func_001FA688(D_L04_00166E00.angle,func_L00_001FF8B0(m->pos.f[0]-D_L04_00166E00.x,m->pos.f[1]-D_L04_00166E00.y))>1.5707964f) {
            m->sort=*(s32 *)(m->metadata+0x10);
            m->flags&=0xFFBE;
            d->visibility_active=0;
            if (d->partner!=-1) { VehicleMoby_2CB800_c2ca420 *other=MOBYS+d->partner; other->flags&=0xFFBE; }
            m->flags|=0x1000;
        } else {
            m->sort=0;
            m->flags|=0x41;
            if (d->partner!=-1) { VehicleMoby_2CB800_c2ca420 *other=MOBYS+d->partner; other->flags|=0x41; }
            m->flags&=0xEFFF;
        }
    }
}
