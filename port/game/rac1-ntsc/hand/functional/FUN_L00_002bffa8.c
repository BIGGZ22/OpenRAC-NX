/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002C12B0.c (games/rac1) through the version
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
/* NON_MATCHING FUN_L00_002bffa8 -- src/overlays/shared/vendor_002C12B0.c
 * Best so far: BYTES 12/5996 (99.8% of the bytes match), checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   `$4=$21` of the following func_L00_002BEBC8 call (REG_DEP_OUTPUT 3234 on insn 3249). Retail's order needs that
 *   ranked last. With the block ending after the spring call (w7a: spring written in both cases, label before
 *   func_L00_002BEBC8) f12 comes first as in retail, but a0 still precedes a1 and cross-jumping does not merge the
 *   (SIZE). Tried without effect: direction nested in the call or as a local, `hero` declared first, a pointer loc
 *   `&data->turn_velocity`, the other prototype order (float before the pointer).
 *   Not in alias form. Declarations to look at when landing: all private (suffix 44); `gravity44` must stay the fi
 *   declaration of D_0015ED70 in the file.
 *   Final check against the destination file as of 2026-10-09 15:30 (others had landed into these files): run 2, B
 */
typedef union { float f[4]; struct { float x,y,z,w; } v; unsigned int q __attribute__((mode(TI))); } V44_c2bffa8;
typedef struct M44_c2bffa8 M44_c2bffa8;
typedef struct { char pad0[0x24]; float base_scale; char pad28[0x1E]; short category; } Class44_c2bffa8;
typedef struct { char pad0[10]; unsigned char radius; char padB[5]; float height; } Aim44_c2bffa8;
typedef struct {
    V44_c2bffa8 target,velocity,previous_position;
    int timer,age;
    M44_c2bffa8 *target_moby,*last;
    float turn_velocity,movement_speed,unknown48;
    int unknown4C;
    float f50,f54,f58,turn_rate;
    M44_c2bffa8 *owner;
    float angle;
    unsigned char mode;
} Doom44_c2bffa8;
struct M44_c2bffa8 {
    char pad0[0x10]; V44_c2bffa8 position;
    unsigned char state; char pad21[3]; Class44_c2bffa8 *cls;
    int unknown28; float scale;
    char pad30[4]; unsigned short flags; char pad36[10];
    V44_c2bffa8 rotation;
    char pad50[2]; unsigned char anim_primary,anim_secondary;
    char pad54[4]; float anim_speed; char pad5C[0x14];
    unsigned char anim_flags; char pad71[7]; Doom44_c2bffa8 *data;
    char pad7C[0x2A]; short class_id;
};
typedef struct {
    V44_c2bffa8 target;
    float f10,f14,f18,gravity,radius,f24,f28;
    unsigned char anim2C,anim2D,anim2E,mode2F;
    float f30,speed,rate,max;
} Config44_c2bffa8;
typedef struct {
    char pad0[0x1EC0]; int damage;
    float f1EC4,f1EC8,f1ECC,turn_velocity,movement_velocity;
    int f1ED8;
    float turn_k,turn_d,turn_max,yawlim;
    char pad1EEC[4]; float f1EF0; char pad1EF4[8];
    float move_k,move_d,move_max;
    int f1F08; char pad1F0C[4]; Config44_c2bffa8 config;
    char pad1F50[0x130]; M44_c2bffa8 *moby;
} Hero44_c2bffa8;
typedef struct { char pad0[0x14]; unsigned char gold; } Order44_c2bffa8;
typedef struct { char pad0[0x18]; M44_c2bffa8 *moby; int tri; V44_c2bffa8 normal,position,reflection; } Hit44_c2bffa8;
extern unsigned char D_0013E533[] NOT_SDA;
extern unsigned char order_bytes44[] __asm__("D_0013E520") NOT_SDA;
extern Hit44_c2bffa8 hit44 __asm__("D_L00_00173E40");
extern M44_c2bffa8 *hit_moby44 __asm__("D_L00_00173E58");
extern V44_c2bffa8 hit_point44[] __asm__("D_L00_00173E70");
extern char D_L00_00177F00[],D_L00_001E9B48[],D_L00_001E9B70[];
extern int pause44 SDATA(D_L00_0015F5C4);
extern float D_0015ED60 MACRO_ADDR;
extern float gravity44 __asm__("D_0015ED70") MACRO_ADDR;
extern void *func_L00_0025A420(void*,int,int);
extern float func_001FA6C0(int),func_001F99C0(float),func_001F9DC8(float),func_001F9DE0(float);
extern float func_001F9B48(void*,void*),func_001F9B80(void*,void*),func_001F9AF0(void*);
extern float func_L00_001FF8B0(float,float),func_001FA688(float,float),func_002132A8(float,float);
extern int func_001FA6D0(float),func_001F9688(int),func_00213260(int);
extern void *func_001153FC(void*,int,unsigned int);
extern void print44(char*) __asm__("func_001E93B0");
extern void anim44(void*,int,int,int) __asm__("func_00212F90");
extern void delete44(void*) __asm__("func_0020C828");
extern int timer44(int*) __asm__("func_001F9740");
extern void func_001F9A10(void*,void*,void*),func_001F9A28(void*,void*,void*),func_001F9A68(void*,void*,float);
extern void func_L00_001FF500(void*,void*,float),func_L00_001FF598(void*,void*,float);
extern int func_L00_001FF600(float,void*,void*);
extern void func_L00_002BEB90(char*),func_L00_002BEBC8(char*),func_L00_002BEC00(char*),func_L00_002BEC48(char*);
extern int func_L00_002BEAB0(char*,int,void*);
extern float func_00213508(void*,int,float);
extern int contact44(void*,void*,void*,void*,void*,void*) __asm__("func_L00_00260400");
extern Aim44_c2bffa8 *aim44(M44_c2bffa8*) __asm__("func_L00_0025C338");
extern int move44(void*,void*,int,int,float,float) __asm__("func_L00_00258AD0");
extern void spring44(void*,float*,float,float,float,float) __asm__("func_L00_00258278");
extern int sphere44(void*,float,int,void*) __asm__("func_L00_001F0D60");
extern int ray44(void*,void*,int,void*,int) __asm__("func_L00_001EFC70");
extern M44_c2bffa8 *find44(void*,void*,void*,void*,int) __asm__("func_L00_002BED90");
extern float steer44(void*,void*,float,float,float,float,float,void*,void*,float) __asm__("func_L00_002BF050");
extern int jump44(void*,void*,void*,int) __asm__("func_L00_002BF9F0");
extern void launch44(void*,void*,void*) __asm__("func_L00_002BF8B0");
extern int state44(void*) __asm__("func_L00_002BFEC8");
extern int land44(void*,void*) __asm__("func_L00_0025B440");
extern int walk44(void*,void*,void*,void*) __asm__("func_L00_00259028");
extern void explode44(void*,void*,void*,float,float,int,int,int,float,float,float,float,int,float,int,int,int,int) __asm__("func_L00_0025E450");
extern int area44(void*,float,int,void*,void*) __asm__("func_L00_001F2868");
extern void spawn44(void*,void*,void*,int,void*,int,int,int,float,float,float) __asm__("func_L00_0025A9F8");
extern int event44(int,int,void*) __asm__("func_0022DA68");

/* Updates an Agent of Doom's movement, target selection and detonation. */
void FUN_L00_002bffa8(M44_c2bffa8 *moby) {
    V44_c2bffa8 scratch[4];
    float clearance;
    Doom44_c2bffa8 *data;
    M44_c2bffa8 *target;
    Aim44_c2bffa8 *aim;
    float scale,speed,z,height,reach,angle,distance;
    int blocked,move_flags,count,result;
    unsigned char old_state;
    if (!moby || !(data=moby->data)) return;
    if (pause44==2) { moby->flags|=0x41; return; }
    moby->flags &= 0xFFBE;
    if (moby->position.v.x<2.0f || moby->position.v.x>1021.0f ||
        moby->position.v.y<2.0f || moby->position.v.y>1021.0f ||
        moby->position.v.z<2.0f || moby->position.v.z>1021.0f) { delete44(moby); return; }
    if (func_L00_0025A420(moby,0x800001,0)) moby->state=11;
    { Order44_c2bffa8 *order=(Order44_c2bffa8*)order_bytes44;
    scale=func_001FA6C0(order->gold)+1.0f;
    moby->scale += (moby->cls->base_scale*scale-moby->scale)*0.05f;
    speed=func_001FA6C0(order->gold)*0.5f+1.0f;
    if (moby->anim_primary==0 || moby->anim_primary==6 || moby->anim_primary==7)
        moby->anim_speed=speed/scale;
    else moby->anim_speed=1.0f;
    }
    if (moby->state!=11) {
        int damage=func_001FA6D0(scale*183.296f)+1;
        Hero44_c2bffa8 *hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        hero->damage=damage;
        hero->f1EF0=0;
        hero->f1EC4=scale*0.198f+0.02f;
        if (moby->state==0) {
            float frame,turn;
            func_001153FC(&hero->config,0,64);
            frame=D_0015ED60;
            hero->f1ECC=hero->f1EC8=scale*0.25f;
            data->movement_speed=frame*0.2f;
            turn=scale*0.01f*frame;
            data->turn_velocity=turn;
            hero->turn_d=0.3f; hero->turn_max=2.7f;
            hero->turn_k=turn; hero->f1ED8=0;
            data->turn_rate=frame*0.066f*speed;
            hero->config.anim2C=2; hero->config.anim2D=3; hero->config.anim2E=4;
            hero->yawlim=3.1415927f;
            hero->move_d=frame*0.01f;
            hero->move_k=frame*0.02f;
            hero->move_max=frame*0.045f;
            hero->config.gravity=gravity44*29.7f;
            hero->config.radius=scale*0.1f;
            hero->config.f24=7.5f; hero->config.f28=11.5f;
            hero->config.speed=frame*0.02f;
            hero->config.rate=frame*0.01f;
            hero->config.max=frame*0.045f;
            data->target_moby=0; moby->state=14; data->timer=0;
            data->turn_rate=D_0015ED60*0.1f*speed;
            func_L00_002BEB90((char*)data);
            func_L00_002BEC00((char*)data);
            if (moby->anim_secondary!=5) anim44(moby,5,0,func_001F9688(5));
        }
        if (moby->state==13) {
            if (!data->owner || data->owner->state==0xFE || data->owner->state==0xFD ||
                moby->position.v.x<2.0f || moby->position.v.x>1021.0f ||
                moby->position.v.y<2.0f || moby->position.v.y>1021.0f ||
                moby->position.v.z<2.0f || moby->position.v.z>1021.0f) { delete44(moby); return; }
            return;
        }
        data->age++;
        if (func_001F9688(3600)<data->age) moby->state=11;
        timer44(&data->timer);
        qcopy(&data->previous_position,&moby->position);
        if (func_L00_002BEAB0((char*)moby,5,0)) {
            if (hit44.moby && hit44.tri<0 && (hit44.moby->flags&0x1000) &&
                hit44.moby->cls && hit44.moby->cls->category==5) {
                moby->state=11;
            } else if (!hit44.moby || hit44.tri<0) {
                        z=moby->position.v.z;
                qcopy_nc(&moby->position,hit_point44);
                if (func_001F99C0(moby->position.v.z-z)>0.6f) moby->position.v.z=z+0.01f;
                qcopy(&data->velocity,hit_point44+1);
                func_L00_001FF598(&data->velocity,&data->velocity,D_0015ED60*0.05f);
                func_L00_001FF600(D_0015ED60*0.05f,&data->velocity,&data->velocity);
                moby->state=8;
                if (data->target_moby) { data->last=data->target_moby; data->timer+=500; }
                if (moby->anim_secondary!=5) anim44(moby,5,0,func_001F9688(5));
            }
        }
        if (moby->position.v.z<0.1f) { delete44(moby); return; }
        if (data->timer>=4001) moby->state=11;
    }
    if (func_00213508(&moby->position,0,0.5f)!=0 && hit_moby44)
        contact44(moby,hit_moby44,&moby->position,&moby->rotation,&moby->position,&moby->rotation);
    switch (moby->state) {
    case 0:
        print44(D_L00_001E9B48); moby->state=1; break;
    case 8: {
        Hero44_c2bffa8 *hero;
        func_001F9A10(&scratch[0],&moby->position,&data->velocity);
        data->velocity.v.z-=gravity44*9.0f;
        func_L00_002BEB90((char*)data);
        hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        blocked=move44(moby,&data->velocity,hero->damage,0,scale*0.7f,hero->f1EF0)&2;
        if (blocked) { data->velocity.v.z=0; func_001F9A68(&data->velocity,&data->velocity,0.5f); }
        func_L00_001FF600(D_0015ED60*0.05f,&data->velocity,&data->velocity);
        if (moby->anim_flags&2) {
            if (!blocked) { if (moby->anim_secondary!=5) anim44(moby,5,0,func_001F9688(5)); }
            else moby->state=2;
        }
        goto Falling;
    }
    case 1: case 9: case 10:
        if (moby->anim_flags&2) moby->state=2;
        goto Contact;
    case 2: case 3: case 4: case 5: {
        target=data->target_moby;
        if (target && target->state!=0xFE && target->state!=0xFD) {
            height=target->position.v.z;
            aim=aim44(data->target_moby); reach=speed;
            if (aim) { reach=speed+func_001FA6C0(aim->radius)*0.125f; height+=aim->height; }
            if (func_001F9B48(&moby->position,&data->target_moby->position)<reach && sphere44(&moby->position,speed,1,moby)) {
                moby->state=11; break;
            }
            if (func_001F9B80(&moby->position,&data->target_moby->position)<reach*0.5f &&
                func_001F99C0(height-(moby->position.v.z+speed*0.1f))<reach*0.5f) {
                func_L00_002BEC00((char*)data);
                launch44(moby,&((Hero44_c2bffa8*)(D_0013E533+0xE1D))->config,&data->target_moby->position);
                func_L00_002BEC48((char*)data);
                qcopy(data,&data->target_moby->position);
                data->turn_rate=D_0015ED60*0.066f*speed;
                if (moby->anim_secondary!=0) anim44(moby,0,0,func_001F9688(2));
                moby->state=6;
            }
        }
        qcopy(&scratch[0],&moby->position);
        scratch[0].v.z+=scale*0.3f;
        if (func_00213260(4)==0) data->target_moby=find44(moby,&scratch[0],&moby->rotation,data->last,1);
        data->turn_rate=D_0015ED60*0.1f*speed;
        moby->state=4;
        if (!data->target_moby) {
            angle=func_002132A8(-3.1415927f,3.1415927f);
            if (func_00213260(4)==0) {
                float step_height=scale*0.35f;
                float range=4.0f;
                float radius=func_001FA6C0(((Hero44_c2bffa8*)(D_0013E533+0xE1D))->damage)*0.0009765625f;
                data->angle=steer44(moby,&moby->position,angle,1.84799564f,range,step_height,4.0f,0,&clearance,radius);
            }
            else clearance=5.0f;
        }
        if (data->target_moby && data->target_moby!=((Hero44_c2bffa8*)(D_0013E533+0xE1D))->moby &&
            (!data->target_moby || data->target_moby->state==0xFE || data->target_moby->state==0xFD || !data->target_moby->cls ||
             data->target_moby->cls->category!=5 || !(data->target_moby->flags&0x1000))) {
            data->target_moby=0; moby->state=2;
            data->turn_rate=D_0015ED60*0.0273f*speed;
            break;
        }
        old_state=moby->state;
        if (!data->target_moby) {
            moby->state=5;
            if (moby->anim_secondary!=6) anim44(moby,6,0,func_001F9688(10));
            data->turn_rate=D_0015ED60*0.0273f*speed;
        } else {
            if (func_00213260(4)==0 && jump44(moby,data,&data->target_moby->position,1)) {
                qcopy(data,&data->target_moby->position);
                data->turn_rate=D_0015ED60*0.066f*speed;
                if (moby->anim_secondary!=0) anim44(moby,0,0,func_001F9688(2));
                moby->state=6; break;
            }
            distance=func_001F9B80(&moby->position,&data->target_moby->position);
            { Hero44_c2bffa8 *hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
            if (data->target_moby==hero->moby) {
                moby->state=5;
                if ((moby->anim_flags&2) && moby->anim_secondary!=6) anim44(moby,6,0,func_001F9688(0));
                data->turn_rate=D_0015ED60*0.0273f*speed;
            }
            }
            if (!(distance<8.0f) && distance>20.0f) {
                moby->state=5;
                if (moby->anim_secondary!=0) anim44(moby,0,0,func_001F9688(10));
                data->turn_rate=D_0015ED60*0.0273f*speed;
            } else {
                moby->state=4;
                if (moby->anim_secondary!=7) anim44(moby,7,0,func_001F9688(10));
                data->turn_rate=D_0015ED60*0.1f*speed;
            }
            if (func_00213260(4)==0) {
                Hero44_c2bffa8 *hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
                V44_c2bffa8 *target_pos=&data->target_moby->position;
                float range=4.0f;
                float radius=func_001FA6C0(hero->damage)*0.0009765625f;
                data->angle=steer44(moby,&moby->position,moby->rotation.v.z,
                    1.84799564f,range,0.35f,4.0f,target_pos,&clearance,radius);
            }
            else clearance=5.0f;
        }
        if (moby->state==4 && old_state==5) {
            moby->state=12;
            if (moby->anim_secondary!=1) anim44(moby,1,0,func_001F9688(10));
            goto Contact;
        }
        data->target.v.y=moby->position.v.y+func_001F9DE0(data->angle)*5.0f;
        data->target.v.x=moby->position.v.x+func_001F9DC8(data->angle)*5.0f;
        data->target.v.z=moby->position.v.z+1.0f;
        func_L00_002BEB90((char*)data);
        { Hero44_c2bffa8 *hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        spring44(moby,&data->movement_speed,data->angle,hero->move_k,hero->move_d,hero->move_max);
        }
        func_001F9A68(&scratch[3],&data->velocity,5.0f);
        scratch[3].v.z=0;
        qcopy(&scratch[1],&moby->position);
        scratch[1].v.z+=scale*0.15f;
        func_001F9A10(&scratch[2],&scratch[1],&scratch[3]);
        if (!ray44(&scratch[1],&scratch[2],6,moby,0) || hit_moby44==data->target_moby ||
            (hit_moby44 && hit_moby44->cls && hit_moby44->cls->category==5)) {
            z=moby->position.v.z;
            move_flags=walk44(moby,&((Hero44_c2bffa8*)(D_0013E533+0xE1D))->damage,data,&data->velocity);
            if (func_001F99C0(moby->position.v.z-z)>1.0f) {
                moby->position.v.z=z;
                if (func_L00_002BEAB0((char*)moby,5,0)) {
                    qcopy(&moby->position,hit_point44);
                    if (func_001F99C0(moby->position.v.z-z)>1.0f) moby->position.v.z=z;
                }
            }
        } else {
            clearance=0; move_flags=2;
            data->velocity.v.x=moby->position.v.x-data->target.v.x;
            data->velocity.v.y=moby->position.v.y-data->target.v.y;
            data->velocity.v.z=0.01f;
            func_L00_001FF500(&data->velocity,&data->velocity,0.1f);
        }
        func_L00_002BEBC8((char*)data);
        if ((move_flags&2) || clearance<4.0f) {
            if (jump44(moby,data,data,0)) {
                data->turn_rate=D_0015ED60*0.066f*speed;
                if (moby->anim_secondary!=0) anim44(moby,0,0,func_001F9688(2));
                moby->state=6; goto Contact;
            }
            if (data->target_moby) { data->last=data->target_moby; data->timer+=500; }
            data->velocity.v.x=moby->position.v.x-data->target.v.x;
            data->velocity.v.y=moby->position.v.y-data->target.v.y;
            data->velocity.v.z=0.01f;
            func_L00_001FF500(&data->velocity,&data->velocity,0.1f);
            moby->state=8;
            if (moby->anim_secondary!=5) anim44(moby,5,0,func_001F9688(5));
            break;
        }
        goto Contact;
    }
    case 6: {
        float heading=func_L00_001FF8B0(data->target.v.x-moby->position.v.x,data->target.v.y-moby->position.v.y);
        float *velocity;
        Hero44_c2bffa8 *hero;
        func_L00_002BEB90((char*)data);
        velocity=&((Hero44_c2bffa8*)(D_0013E533+0xE1D))->movement_velocity;
        hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        spring44(moby,velocity,heading,2.0f*hero->move_k,0.5f*hero->move_d,2.0f*hero->move_max);
        func_L00_002BEBC8((char*)data);
        if (func_001FA688(moby->rotation.v.z,heading)<0.08726646f) {
            anim44(moby,hero->config.anim2C,0,func_001F9688(3));
            moby->state=7;
        }
        goto Contact;
    }
    case 7:
        z=moby->position.v.z;
        func_L00_002BEC00((char*)data);
        result=land44(moby,&((Hero44_c2bffa8*)(D_0013E533+0xE1D))->config);
        if (result==4) {
            moby->state=result;
            if (moby->anim_secondary!=7) anim44(moby,7,0,func_001F9688(3));
            data->turn_rate=D_0015ED60*0.1f*speed;
        }
        func_L00_002BEC48((char*)data);
        func_001F99C0(moby->position.v.z-z);
        goto Contact;
    case 11: {
        Order44_c2bffa8 *order;
        if (!(moby->position.v.x<2.0f) && !(moby->position.v.x>1021.0f) &&
            !(moby->position.v.y<2.0f) && !(moby->position.v.y>1021.0f) &&
            !(moby->position.v.z<2.0f) && !(moby->position.v.z>1021.0f)) {
            qcopy(&scratch[1],&moby->position);
            order=(Order44_c2bffa8*)order_bytes44;
            scratch[2].q=0; scratch[2].v.z=1.0f; scratch[2].v.w=1.0f;
            explode44(moby,&scratch[2],&moby->position,0.0f,0.0f,5,2,4,2.0f*speed,speed,9.0f,1.0f,-1,15.0f,1,5,-1,order->gold);
            scratch[1].v.z+=scale*0.5f;
            count=area44(&moby->position,speed,0x15,moby,0);
            scratch[3].q=moby->position.q;
            spawn44(moby,&scratch[3],D_L00_00177F00,count,0,0x10000,2,3,3.0f,1.0f,1.0f);
            if (order->gold) event44(4,0,moby); else event44(0,0,moby);
        }
    }
    delete44(moby);
    return;
    case 12:
        if (moby->anim_flags&2) {
            moby->state=4;
            if (moby->anim_secondary!=7) anim44(moby,7,0,func_001F9688(10));
            data->turn_rate=D_0015ED60*0.1f*speed;
        }
        break;
    case 13:
        print44(D_L00_001E9B70); break;
    case 14: {
        Hero44_c2bffa8 *hero;
        func_001F9A10(&scratch[1],&moby->position,&data->velocity);
        if (sphere44(&moby->position,scale*0.5f,1,moby) && hit44.moby && hit44.moby->class_id==186) {
            func_001F9A28(&scratch[1],&hit44.position,&moby->position);
            distance=func_001F9AF0(&scratch[1]);
            scratch[1].v.z=0;
            func_L00_001FF500(&scratch[1],&scratch[1],distance*1.1f);
            func_001F9A10(&moby->position,&moby->position,&scratch[1]);
        }
        data->velocity.v.z-=gravity44*9.0f;
        func_L00_002BEB90((char*)data);
        hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        blocked=move44(moby,&data->velocity,hero->damage,0,scale*0.7f,hero->f1EF0)&2;
        if (blocked) { data->velocity.v.z=0; func_001F9A68(&data->velocity,&data->velocity,0.5f); }
        func_L00_001FF598(&data->velocity,&data->velocity,D_0015ED60*0.05f);
        if (!blocked) { if (moby->anim_secondary!=5) anim44(moby,5,0,func_001F9688(5)); }
        else moby->state=2;
    }
Falling: {
        float direction=func_L00_001FF8B0(data->velocity.v.x,data->velocity.v.y);
        Hero44_c2bffa8 *hero=(Hero44_c2bffa8*)(D_0013E533+0xE1D);
        spring44(moby,&data->turn_velocity,direction,
            hero->turn_k,hero->turn_d,hero->turn_max);
        func_L00_002BEBC8((char*)data);
    }
Contact:
        moby->state=state44(moby);
        break;
    case 15: moby->state=0; break;
    default: moby->state=0; break;
    }
    if (moby->position.v.x<2.0f || moby->position.v.x>1021.0f ||
        moby->position.v.y<2.0f || moby->position.v.y>1021.0f ||
        moby->position.v.z<2.0f || moby->position.v.z>1021.0f) delete44(moby);
    return;
}
