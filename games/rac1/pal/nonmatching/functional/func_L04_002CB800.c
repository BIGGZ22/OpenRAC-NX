/* functional: high. Same reads, writes and calls as retail; the only differences tools/equiv.py reports are the three stores of 0.0f (turn velocity, speed), which retail writes from a zeroed $f20 and this build writes as integer zero, the same 32 bits in memory. */
/* func_L04_002CB800 (level 4, Eudora): update of the generic vehicle mobys (written from the retail
 * instructions; not a match). On the first call (state 0) it either steps back the index of its path
 * (D_L04_001B0930) when near its point or calls func_001E9730 with its index, sets shield and alpha and
 * remembers its start speed. Then, unless the class is 0x1E4, it runs a
 * four-phase drive: 0 accelerates back to the start speed and waits until func_00215570 accepts the
 * position for one of its four ids,
 * 1 turns and drives to the triggered path point, 2 waits, 3 returns to the start position and angle.
 * It sets up (state 0) or spins (later) the matrices at data+0x180/+0x1C0 for some classes, carries the partner
 * moby (classes 0x1E0, 0x1EE), and when func_L00_0025B478(m, 0x210000, 0) is nonzero (presumably hit)
 * counts it in D_0015EEF8 (a one-off event at 10), explodes with debris and sound depending on the class, spawns the class's pieces
 * (func_L00_00265050), and, while its visibility timer runs, keeps it drawn (flags 0x41) until the timer
 * ends with the camera facing more than 90 degrees away, then restores its sort value and flags. */
#include "common.h"

typedef int VehicleWide_2CB800 __attribute__((mode(TI)));
typedef union { VehicleWide_2CB800 quad; float f[4]; } VehicleVec_2CB800;
typedef union { s32 id; u8 token; } VehicleTrigger_2CB800;
typedef struct {
    char pad0[0x60];
    char motion[0x14]; s32 path_id;
    char pad78[4]; float turn_velocity;
    char pad80[0x7C]; float speed;
    char pad100[0x1C]; s32 moving_timer;
    char pad120[0x18]; float start_speed;
    char pad13C[4]; float angle0,angle1;
    u8 selected, phase; s16 phase_timer;
    float start_angle; VehicleTrigger_2CB800 paths[4];
    VehicleVec_2CB800 start_pos;
    s32 visibility_timer, visibility_active, shield, partner;
    char matrix0[0x40],matrix1[0x40];
} VehicleData_2CB800;
typedef struct {
    char pad0[0x10]; VehicleVec_2CB800 pos;
    u8 state; char pad21[3]; char *metadata;
    char pad28[8]; u8 alpha; char pad31; s16 shield; u16 flags;
    char pad36[0xA]; VehicleVec_2CB800 rot;
    char pad50[0x28]; VehicleData_2CB800 *data;
    char pad7C[0x18]; s32 sort;
    char pad98[0xC]; u8 draw_alpha; char padA5; s16 cls;
    char padA8[0x18]; char matrix[0x40];
} VehicleMoby_2CB800;
typedef struct { char pad[0x30]; VehicleVec_2CB800 pos; char pad40[0x38]; float angle; char pad7C[4]; } VehiclePath_2CB800;
typedef struct { VehicleVec_2CB800 vec; VehicleMoby_2CB800 *owner; s32 flags, unused; float strength; s32 result; } VehicleQuery_2CB800;
extern s32 *D_L04_001B0930[];
extern VehicleMoby_2CB800 *vehicle_mobys_2CB800 __asm__("D_L04_00160058") MACRO_ADDR;
extern VehiclePath_2CB800 *vehicle_paths_2CB800 __asm__("D_L04_0016016C") MACRO_ADDR;
extern char D_L04_001DC110[];
extern s32 D_0015EEF8 MACRO_ADDR;
typedef struct { char pad[6]; u8 flag; } VehicleWeapon_2CB800;
extern VehicleWeapon_2CB800 vehicle_weapon_flags_2CB800 __asm__("D_0013D510");
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;
extern float D_0015EE60 MACRO_ADDR;
extern float D_L04_0015F660[] MACRO_ADDR;
typedef struct { char pad[0x140]; float x,y,z; char pad14C[12]; float angle; } VehicleCamera_2CB800;
extern VehicleCamera_2CB800 vehicle_camera_2CB800 __asm__("D_L04_00166E80");
extern float func_001F9D10(void *,void *);
extern s32 func_001FA898(float);
extern s32 func_001E9730_2CB800(char *, s32, s32) __asm__("func_001E9730");
extern void vehicle_base_update_2CB800(VehicleMoby_2CB800 *) __asm__("func_L01_002F6540");
extern void func_001F9938(void *);
extern s32 func_00215570(void *,s32);
extern void vehicle_turn_2CB800(void *,void *,float,float,float,float) __asm__("func_L00_002592B0");
extern float func_001F9D48(void *,void *);
extern float func_001FA850(float,float);
extern s32 func_001F9850(s32);
extern void func_001F9BF0(void *,void *,void *);
extern void func_L00_001FF4B0(void *,void *,float);
extern void func_001F9BD8(void *,void *,void *);
extern float func_001F9CB8(void *);
extern void func_0020D960(void *,s32,void *);
extern float func_001FA748(float,float);
extern void func_L00_001FFED8(void *,s32,float);
extern void func_001F9EC0(void *,void *,void *);
extern void func_L00_002514B8(void *);
extern void func_L00_00251E30(void *);
extern void func_0020EEE8(void *);
extern char *func_L00_0025B478(void *,s32,s32);
extern s32 func_0022EE28(s32,s32,s32);
extern void func_L00_00264DB8(s32,s32);
extern void func_001F9BC0(void *);
extern void func_L00_001F2BE8(void *,float,s32,void *,void *);
extern void func_L00_0025F4A8_alt(void *,void *,void *,float,float,s32,s32,s32,float,float,float,float,s32,float,s32,s32,s32,s32) __asm__("func_L00_0025F4A8");
extern void vehicle_sound_2CB800(s32,s32,void *) __asm__("func_0022ED80");
extern void func_L00_00250800(void *,s32,void *);
extern void func_L00_00260108(void *,void *,s32,float,float);
extern void func_001F9C30(void *,void *,float);
extern void func_L00_00265050(void *,s32,void *,void *,s32,s32,float,void *,void *,void *);
extern s32 func_001F9908(s32 *);
extern float func_L00_001FF860(float,float);

typedef float VehicleArr_2CB800[4] __attribute__((aligned(16)));
typedef struct { char pad[0x14]; s32 path_id; } VehicleSub_2CB800;

/* Moves generic vehicle mobys along triggered paths and updates their attached parts, damage effects and visibility. */
void func_L04_002CB800(VehicleMoby_2CB800 *m) {
    VehicleVec_2CB800 old, delta;
    VehicleData_2CB800 *d=m->data;
    u8 state=m->state;
    VehicleSub_2CB800 *sub;
    VehicleTrigger_2CB800 *trigger;
    float zero;
    qcopy(&old,&m->pos);
    if (!d) return;
    sub=(VehicleSub_2CB800 *)d->motion;
    if (state==0) {
        if (sub->path_id!=-1) {
            s32 *path=D_L04_001B0930[sub->path_id];
            if (func_001F9D10((char *)path+0x10,(char *)path+(*path<<4))<0.5f) --*path;
        } else {
            func_001E9730_2CB800(D_L04_001DC110,m->cls,func_001FA898((float)((s32)((char *)m-(char *)vehicle_mobys_2CB800)>>8)));
        }
        m->shield=0x7F;
        m->alpha=0xFF;
        d->start_speed=d->speed;
        d->shield=m->shield;
    }
    zero=0.0f;
    vehicle_base_update_2CB800(m);
    if (m->cls!=0x1E4) {
        func_001F9938(&d->phase_timer);
        switch (d->phase) {
        case 0: {
            s32 i;
            if (d->speed<d->start_speed) {
                d->speed+=D_0015EE70*20.0f;
                if (d->speed>d->start_speed) d->speed=d->start_speed;
            }
            for (i=0,trigger=d->paths;i<4;i++,trigger++) {
                if (d->phase_timer==0 && d->paths[i].id!=-1 && func_00215570(&m->pos,d->paths[i].id)) {
                    qcopy(&d->start_pos,&m->pos);
                    d->selected=trigger->token;
                    d->phase=1;
                    d->start_angle=m->rot.f[2];
                    d->turn_velocity=0;
                    m->state=3;
                    d->moving_timer=func_001F9850(0x8CA0);
                    break;
                }
            }
            break;
        }
        case 1:
            vehicle_turn_2CB800(m,&d->turn_velocity,vehicle_paths_2CB800[d->selected].angle,D_0015EE70*1.0471976f,D_0015EE70*3.1415927f,D_0015EE6C*3.1415927f);
            if (func_001F9D48(&m->pos,&vehicle_paths_2CB800[d->selected].pos)<1.0f &&
                func_001FA850(m->rot.f[2],vehicle_paths_2CB800[d->selected].angle)<0.17453292f) {
                d->phase=2;
                d->phase_timer=func_001F9850(300);
            }
            func_001F9BF0(&delta,&vehicle_paths_2CB800[d->selected].pos,&m->pos);
            func_L00_001FF4B0(&delta,&delta,D_0015EE6C*3.0f);
            func_001F9BD8(&m->pos,&m->pos,&delta);
            break;
        case 2:
            if (d->phase_timer==0) { d->turn_velocity=zero; d->phase=3; }
            break;
        case 3: {
            float length,step;
            vehicle_turn_2CB800(m,&d->turn_velocity,d->start_angle,D_0015EE70*1.0471976f,D_0015EE70*3.1415927f,D_0015EE6C*3.1415927f);
            if (func_001F9D48(&m->pos,&d->start_pos)<1.0f && func_001FA850(m->rot.f[2],d->start_angle)<0.034906585f) {
                d->phase=0;
                d->phase_timer=func_001F9850(600);
                d->turn_velocity=zero;
                d->speed=zero;
                d->moving_timer=0;
            }
            func_001F9BF0(&delta,&d->start_pos,&m->pos);
            length=func_001F9CB8(&delta);
            func_L00_001FF4B0(&delta,&delta,D_0015EE6C*3.0f>length ? length : D_0015EE6C*3.0f);
            func_001F9BD8(&m->pos,&m->pos,&delta);
            break;
        }
        }
    }
    if (state==0) {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:case 19:case 20:case 28:case 29:case 32:case 89:
            func_0020D960(m,0,d->matrix0);d->angle0=0;break;
        case 22:case 27:
            func_0020D960(m,0,d->matrix0);d->angle0=0;
            func_0020D960(m,1,d->matrix1);d->angle1=0;break;
        }
    } else {
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 0:case 14:
            d->angle0=func_001FA748(d->angle0,D_0015EE6C*12.566371f);
            func_L00_001FFED8(d->matrix0+0x10,0,d->angle0);break;
        case 19:case 20:case 28:case 29:case 32:case 89:
            d->angle0=func_001FA748(d->angle0,D_0015EE6C*12.566371f);
            func_L00_001FFED8(d->matrix0+0x10,2,d->angle0);break;
        case 22:case 27:
            d->angle0=func_001FA748(d->angle0,D_0015EE6C*12.566371f);
            func_L00_001FFED8(d->matrix0+0x10,2,d->angle0);
            d->angle1=func_001FA748(d->angle1,D_0015EE6C*12.566371f);
            func_L00_001FFED8(d->matrix1+0x10,2,d->angle1);break;
        }
    }
    if (d->partner!=-1) {
        VehicleMoby_2CB800 *other=vehicle_mobys_2CB800+d->partner;
        other->shield=0x7F;
        switch (m->cls) {
        case 0x1E0:
            delta.quad=0;
            delta.f[0]=0.5f;delta.f[2]=-5.0f;
            func_001F9EC0(&delta,&delta,m->matrix);
            func_001F9BD8(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            other->rot.f[1]=func_001FA748(other->rot.f[1],1.5707964f);
            func_L00_002514B8(other);func_L00_00251E30(other);func_0020EEE8(other);
            other->flags|=6;
            break;
        case 0x1EE:
            delta.quad=0;delta.f[2]=-2.0f;
            func_001F9EC0(&delta,&delta,m->matrix);
            func_001F9BD8(&other->pos,&m->pos,&delta);
            qcopy(&other->rot,&m->rot);
            func_L00_002514B8(other);func_L00_00251E30(other);func_0020EEE8(other);
            other->flags|=6;
            break;
        }
    }
    if (func_L00_0025B478(m,0x210000,0)) {
        if (++D_0015EEF8>=10 && vehicle_weapon_flags_2CB800.flag==0) {
            vehicle_weapon_flags_2CB800.flag=1;
            func_0022EE28(1,0,0);func_L00_00264DB8(0x53DB,-1);
        }
        switch ((s16)((u16)m->cls-0x1D2)) {
        case 19:case 20: {
            {
                VehicleQuery_2CB800 query;
                query.flags=0x810000;query.strength=1;query.owner=m;
                func_001F9BC0(&query.vec);query.result=0;
                func_L00_001F2BE8(&m->pos,4.0f,16,m,&query);
            }
        }
        case 0:case 14:case 22:case 24:case 28:case 29:case 32:case 89: {
            {
                VehicleArr_2CB800 none;
                delta.quad=0;delta.f[2]=0.75f;
                func_001F9BC0(none);
                func_001F9BD8(&delta,&delta,&m->pos);
                func_L00_0025F4A8_alt(m,none,&delta,0.0f,0.0f,10,3,4,2,1,100000,3,-1,15,1,1,-1,0);
                if (m->cls==0x1F2 || m->cls==0x22B) vehicle_sound_2CB800(0,0,m);
                else if (m->cls!=0x1E6) vehicle_sound_2CB800(1,0,m);
                d->visibility_timer=func_001F9850(600);d->visibility_active=1;
            }
            break;
        }
        case 27: {
            {
                VehicleArr_2CB800 offset={0.0f,0.0f,0.75f};
                VehicleArr_2CB800 none;
                VehicleQuery_2CB800 query2;
                func_001F9BC0(none);
                func_L00_00250800(m,0,&delta);
                func_001F9BD8(&delta,&delta,offset);
                func_L00_00260108(m,&delta,-1,1.0f,13.0f);
                func_L00_00250800(m,1,&delta);
                func_001F9BD8(&delta,&delta,offset);
                func_L00_00260108(m,&delta,-1,1.0f,13.0f);
                func_001F9BD8(&delta,offset,&m->pos);
                func_L00_0025F4A8_alt(m,none,&delta,0.0f,0.0f,20,5,4,2,1,100000,3,-1,15,1,1,-1,0);
                vehicle_sound_2CB800(1,0,m);
                query2.flags=0x810000;query2.strength=2;query2.owner=m;
                func_001F9BC0(&query2.vec);query2.result=0;
                func_L00_001F2BE8(&m->pos,6.0f,16,m,&query2);
                d->visibility_timer=func_001F9850(600);d->visibility_active=1;
            }
            break;
        }
        }
        func_001F9C30(&delta,m->matrix,D_0015EE60*0.075f);
        delta.f[2]+=D_0015EE60*0.08f;
        if (m->cls==0x1D2) {
            func_L00_00265050(m,0x63F,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x641,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x642,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1E0) {
            func_L00_00265050(m,0x642,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x643,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x644,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1E6) {
            s32 i;
            func_L00_00265050(m,0x645,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            for (i=0;i<6;i++) {
            func_L00_00265050(m,0x646,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
        }
        else if (m->cls==0x1EA) {
            func_L00_00265050(m,0x647,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x648,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x649,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1EE) {
            func_L00_00265050(m,0x64D,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x64E,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1EF) {
            func_L00_00265050(m,0x64F,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x650,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x651,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x658,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1F2) {
            func_L00_00265050(m,0x652,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x653,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x654,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
        }
        else if (m->cls==0x1ED) {
            s32 j,k;
            func_L00_00265050(m,0x793,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            func_L00_00265050(m,0x794,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            for (j=0;j<3;j++) {
            func_L00_00265050(m,0x795,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
            for (k=0;k<3;k++) {
            func_L00_00265050(m,0x796,&m->pos,&m->rot,func_001F9850(90),0,D_0015EE70*12.0f,&delta,D_L04_0015F660,D_L04_0015F660);
            }
        }
    }
    m->draw_alpha=0xFF;
    if (d->visibility_active) {
        if (func_001F9908(&d->visibility_timer) &&
            func_001FA850(vehicle_camera_2CB800.angle,func_L00_001FF860(m->pos.f[0]-vehicle_camera_2CB800.x,m->pos.f[1]-vehicle_camera_2CB800.y))>1.5707964f) {
            m->sort=*(s32 *)(m->metadata+0x10);
            m->flags&=0xFFBE;
            d->visibility_active=0;
            if (d->partner!=-1) { VehicleMoby_2CB800 *other=vehicle_mobys_2CB800+d->partner; other->flags&=0xFFBE; }
            m->flags|=0x1000;
        } else {
            m->sort=0;
            m->flags|=0x41;
            if (d->partner!=-1) { VehicleMoby_2CB800 *other=vehicle_mobys_2CB800+d->partner; other->flags|=0x41; }
            m->flags&=0xEFFF;
        }
    }
}
