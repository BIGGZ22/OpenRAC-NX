/* Carried from the PAL decompilation's nonmatching/shared/func_L00_002DF410.c (games/rac1) through the version
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
/* What the PAL file this was written in declares before it (its unit, and its own file). */
extern short D_L00_00161B54;
extern short D_L00_00161B58;
typedef unsigned int Q41_c2ddf60 __attribute__((mode(TI)));
typedef union { struct { float x,y,z,w; } f; Q41_c2ddf60 q; } V41_c2ddf60 __attribute__((aligned(16)));
typedef struct {
    u8 pad0[0x34]; u16 flags; u8 pad36[0x8A]; V41_c2ddf60 basis[3];
} M41_c2ddf60;
typedef struct {
    u8 pad0[0x640]; V41_c2ddf60 matrix[3]; u8 pad670[0x1A10];
    M41_c2ddf60 *moby; int camera_mode; u8 pad2088[0x240]; s16 special;
} Hero41_c2ddf60;
typedef struct Particle41_c2ddf60 {
    u8 pad0[0xA]; s16 life; float size; V41_c2ddf60 pos;
    float unknown20; int mode; u8 pad28[2]; u8 r,g; u8 pad2C[4];
    V41_c2ddf60 velocity; float acceleration;
} Particle41_c2ddf60;
typedef struct {
    s16 segment,angle; float distance,phase,phase_step,speed; Particle41_c2ddf60 *particle;
} Entry41_c2ddf60;
extern char D_0013E533[] NOT_SDA;
extern char D_L00_00166FD0[];
extern float D_L00_00161B84 MACRO_ADDR;
extern char D_L00_001E5F40[];
extern char D_L00_001E5F30[];
extern char D_L00_00173E60[];
extern short D_L00_001E6008[];
extern V41_c2ddf60 D_L00_00161B60 MACRO_ADDR;
extern V41_c2ddf60 D_L00_00161B70 MACRO_ADDR;
extern float D_L00_00161B80 MACRO_ADDR;
extern int tick41 SDATA(D_L00_00161B54);
extern float wave41 SDATA(D_L00_00161B58);
extern float motion_scale41 SDATA(D_L00_00161B84);
extern V41_c2ddf60 D_L00_001E4B40[];
extern V41_c2ddf60 D_L00_001E6020[];
extern V41_c2ddf60 D_L00_001E5EE0[];
extern float D_L00_001E5FE0[];
extern Entry41_c2ddf60 entries41[] __asm__("D_L00_001E4C80");
extern float D_0015ED60 MACRO_ADDR;
extern void func_L00_002DDCB8(void *,void *);
extern void func_001F9A68(void *,void *,float);
extern void func_L00_001FF500(void *,void *,float);
extern void func_001F9A10(void *,void *,void *);
extern int func_L00_001EFC70(void *,void *,int,int,int);
extern float func_001F9B80(void *,void *);
extern int func_001FA6D0(float);
extern void func_001F9CF8(void *,void *,void *);
extern int func_001F9688(int);
extern float func_001FA6C0(int);
extern float func_002132A8(float,float);
extern float func_001FA580(float,float);
extern float func_001FA5C8(float,float);
extern void func_00214890(void *,void *,float,void *);
extern void func_001F9AD8(void *,void *,void *);
extern void func_001F9A28(void *,void *,void *);
extern float func_001F9AB0(void *,void *);
extern float func_001F9AF0(void *);
extern float func_001F9DF8(float);
extern float func_L00_001EB328(void *,float,float,float,float,float);
extern float func_001F99C0(float);
extern void func_001F9A40(void *,void *,void *,float);
extern float func_001F9DE0(float);
extern void func_L00_002DD8E8(void);
extern void func_001F4600(void (*)(void),int);
extern int func_00213260(int);
extern Particle41_c2ddf60 *new_particle41(void *,int,void *,int,float,float,float,float) __asm__("func_L00_0026D000");
extern void func_001F99F8(void *);
extern void func_L00_00267A08(void *);
extern void func_L00_0025B6B8(void *,void *,void *,float);
static __inline__ void clear_vectors41_c2ddf60(V41_c2ddf60 *dst_a,V41_c2ddf60 *dst_b) {
    register V41_c2ddf60 a,b;
    a.q=0;
    b.q=0;
    dst_a->q=a.q;
    dst_b->q=b.q;
}



/* Form the Suck Cannon's bending suction tube and update its particles. */
void FUN_L00_002ddf60(void *moby,void *position) {
    V41_c2ddf60 step,up,orient[3],ray_end,start,end,axis,tmp_a,tmp_b,
        cross_b,cross_a,basis,tmp_c,motion,tmp_d,tmp_e,interp,cross_c,basis_c,tmp_f;
    V41_c2ddf60 *orientation;
    int i,j,n,count,sparks,segment,ring,next_ring;
    float t,angle,turn,delta,dot,length,radius,phase,amount,speed;
    Particle41_c2ddf60 *p;
    V41_c2ddf60 *anchor,*previous,*source;
    Entry41_c2ddf60 *entry;


    func_L00_002DDCB8(moby,position);
    if((((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->camera_mode==30 || ((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->camera_mode==1) && (((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->moby->flags&1)) {
        qcopy(&orient[0],D_L00_00166FD0);
        qcopy(&orient[1],D_L00_00166FD0+16);
        qcopy(&orient[2],D_L00_00166FD0+32);
        orientation=&orient[0];
    } else if(((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->special==1) {
        func_001F9A68(&orient[0],&((M41_c2ddf60 *)moby)->basis[1],-1.0f);
        { M41_c2ddf60 *m=moby;
        qcopy_nc(&orient[1],&m->basis[0]);
        qcopy_nc(&orient[2],&m->basis[2]); }
        orientation=&orient[0];
    } else {
        qcopy(&orient[0],&((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->matrix[0]);
        qcopy(&orient[1],&((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->matrix[1]);
        qcopy(&orient[2],&((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->matrix[2]);
        orientation=&orient[0];
    }
    func_L00_001FF500(&step,orientation,D_L00_00161B84);
    func_L00_001FF500(&up,&orient[2],1.0f);
    qcopy(D_L00_001E5F40,position);
    func_L00_001FF500(&ray_end,orientation,D_L00_00161B84*10.0f);
    func_001F9A10(&ray_end,&ray_end,position);
    if(func_L00_001EFC70(position,&ray_end,20,(int)((Hero41_c2ddf60 *)(D_0013E533+0xE1D))->moby,0)) {
        int n,i;
        n=func_001FA6D0(func_001F9B80(D_L00_00173E60,position)/D_L00_00161B84);
        for(i=n;i<10;i++) {
            if(D_L00_001E6008[i]>64)D_L00_001E6008[i]=64;
            D_L00_001E6008[i]=(u16)D_L00_001E6008[i]-4;
            if(D_L00_001E6008[i]<0)D_L00_001E6008[i]=0;
        }
        for(i=n-1;i>=0;i--)D_L00_001E6008[i]=D_L00_001E6008[i+1]+32;
    } else {
        int i;
        D_L00_001E6008[0]=(u16)D_L00_001E6008[0]+4;
        for(i=1;i<10;i++)D_L00_001E6008[i]=D_L00_001E6008[i-1]-32;
    }
    func_001F9CF8(&start,&D_L00_00161B60,orientation);
    func_001F9A10(&start,&start,&step);
    func_001F9CF8(&end,&D_L00_00161B70,orientation);
    func_001F9A10(&end,&end,&step);
    t=(float)tick41/func_001FA6C0(func_001F9688(60));
    tick41++;
    step.f.x=start.f.x+(end.f.x-start.f.x)*t;
    step.f.y=start.f.y+(end.f.y-start.f.y)*t;
    step.f.z=start.f.z+(end.f.z-start.f.z)*t;
    if(tick41>=func_001F9688(60)) {
        qcopy(&D_L00_00161B60,&D_L00_00161B70);
        func_L00_001FF500(&axis,orientation,D_L00_00161B84);
        clear_vectors41_c2ddf60(&tmp_a,&tmp_b);
        tmp_a.f.z=0.1f; tmp_a.f.w=1.0f;
        tmp_b.f.x=1.0f; tmp_b.f.w=1.0f;
        angle=func_001FA580(D_L00_00161B80,func_002132A8(45.0f,315.0f)*0.0174532924f);
        func_00214890(&D_L00_00161B70,&tmp_a,angle,&tmp_b);
        D_L00_00161B80=angle;
        tick41=0;
    }
    wave41=func_001FA580(wave41,0.1f);
    qcopy(&axis,&orient[2]);
    qcopy(&basis,&axis);
    func_001F9AD8(&cross_a,&step,&basis);
    func_L00_001FF500(&cross_a,&cross_a,1.0f);
    func_001F9AD8(&cross_b,&basis,&cross_a);
    { int i;
    for(i=19;i>=0;i--) {
        func_001F9A68(&tmp_a,&D_L00_001E4B40[19-i],0.1f);
        func_001F9CF8(&tmp_a,&tmp_a,&cross_b);
        func_001F9A10(&D_L00_001E6020[19-i],D_L00_001E5F40,&tmp_a);
    }
    }
    { int i;
    for(i=1;i<10;i++) {
        anchor=(V41_c2ddf60 *)(D_L00_001E5F40+i*16);
        previous=(V41_c2ddf60 *)(D_L00_001E5F30+i*16);
        func_001F9A28(&tmp_a,anchor,previous);
        qcopy(&tmp_b,&step);
        func_001F9AD8(&tmp_c,&tmp_b,&tmp_a);
        dot=func_001F9AB0(&tmp_a,&tmp_b);
        angle=1.5707964f-func_001F9DF8(dot/(func_001F9AF0(&tmp_a)*D_L00_00161B84));
        turn=func_L00_001EB328(&D_L00_001E5FE0[i],0.0f,angle,0.06f,0.2f,0.0f);
        angle=func_001FA5C8(angle,turn);
        if(func_001F99C0(angle)>0.31415927f)turn=func_001FA580(turn,func_001FA5C8(angle,0.31415927f));
        func_00214890(&motion,&tmp_a,turn,&tmp_c);
        func_L00_001FF500(&tmp_d,&motion,motion_scale41);
        func_001F9A40(&motion,&motion,&tmp_d,1.0f);
        { float len=func_001F9AF0(&motion);
        if(len>2.0f)func_001F9A68(&motion,&motion,2.0f/len); }
        amount=(float)i;
        if(i>=6)func_001F9A40(&motion,&motion,&tmp_a,amount/func_001FA6C0(10)+0.0f);
        func_001F9A10(anchor,previous,&motion);
        qcopy(&tmp_e,&step);
        qcopy(&basis_c,&axis);
        func_L00_001FF500(&interp,&tmp_e,1.0f);
        func_001F9AD8(&cross_c,&interp,&basis_c);
        func_L00_001FF500(&cross_c,&cross_c,1.0f);
        func_001F9AD8(&basis_c,&cross_c,&interp);
        qcopy(&axis,&basis_c);
        radius=amount*0.3f;
        phase=amount*0.7f;
        { int j;
        for(j=0;j<20;j++) {
            angle=radius+0.1f;
            angle+=func_001F9DE0(func_001FA580(wave41+phase,0.0f))*0.1f;
            func_001F9A68(&tmp_f,(char *)D_L00_001E4B40+j*16,angle);
            func_001F9CF8(&tmp_f,&tmp_f,&interp);
            func_001F9A10((char *)D_L00_001E6020+i*320+j*16,D_L00_001E5F40+i*16,&tmp_f);
        }
        }
        func_L00_001FF500(&step,&motion,D_L00_00161B84);
    }
    }
    func_001F4600(func_L00_002DD8E8,(int)moby);
    count=0; sparks=0;
    for(i=0;i<200;i++) {
        if(!entries41[i].particle && count<5) {
            for(segment=9;segment>=0 && D_L00_001E6008[segment]<64;segment--);
            if(segment>0) {
                if(segment>=2 && sparks<2) {
                    ring=func_00213260(20);
                    amount=func_002132A8(0.2f,1.0f);
                    source=&D_L00_001E6020[ring];
                    func_001F9A40(&tmp_a,source,source+segment*20,amount);
                    func_001F9A40(&tmp_c,D_L00_001E5F40,D_L00_001E5F40+segment*16,amount);
                    speed=func_002132A8(60000.0f,180000.0f);
                    p=new_particle41(&tmp_a,func_00213260(5)+1,&tmp_b,0x404040,0.0f,1.0f,1.03f,speed);
                    if(p) {
                        func_001F9A28(&motion,&tmp_a,&tmp_c);
                        sparks++;
                        func_L00_001FF500(&p->velocity,&motion,0.01f);
                        p->velocity.f.z+=0.02f;
                    }
                }
                qcopy(&tmp_a,D_L00_001E5F40+segment*16);
                func_001F99F8(&tmp_b);
                speed=func_002132A8(60000.0f,180000.0f);
                p=new_particle41(&tmp_a,6,&tmp_b,0x7F404040,0.0f,1.0f,1.0f,speed);
                entries41[i].particle=p;
                if(p) { p->life=200; p->mode=2; count++; p->r=127; p->g=200; }
                entry=&entries41[i];
                entries41[i].segment=segment;
                entries41[i].distance=0;
                entries41[i].phase=0;
                entries41[i].angle=func_00213260(20);
                entries41[i].phase_step=func_002132A8(0.1f,0.8f);
                entries41[i].speed=func_002132A8(0.1f,0.4f);
            }
        }
        if(entries41[i].particle) {
            entry=&entries41[i];
            func_001F9A28(&tmp_a,D_L00_001E5F30+entries41[i].segment*16,D_L00_001E5F40+entries41[i].segment*16);
            length=func_001F9AF0(&tmp_a);
            entries41[i].distance+=entries41[i].speed;
            (entries41[i].particle)->life=200;
            if(length<entries41[i].distance) {
                entries41[i].segment=(u16)entries41[i].segment-1;
                if(entries41[i].segment<=0) { func_L00_00267A08(entries41[i].particle); entries41[i].particle=0; }
                else {
                    entries41[i].distance-=length;
                    func_001F9A28(&tmp_a,D_L00_001E5F30+entries41[i].segment*16,D_L00_001E5F40+entries41[i].segment*16);
                    length=func_001F9AF0(&tmp_a);
                }
            }
                if(entries41[i].particle) {
                entry=&entries41[i];
                amount=entries41[i].distance/length;
                if(entries41[i].segment==2)(entries41[i].particle)->life=100;
                else if(entries41[i].segment<2) {
                    (entries41[i].particle)->life=func_001FA6D0((1.0f-amount)*100.0f);
                    if((entries41[i].particle)->life<61)(entries41[i].particle)->life=60;
                }
                entry=&entries41[i];
                if(entries41[i].segment==1) {
                    func_001F9A28(&tmp_b,position,&(entries41[i].particle)->pos);
                    func_L00_001FF500(&tmp_b,&tmp_b,entries41[i].speed);
                    func_001F9A10(&(entries41[i].particle)->pos,&(entries41[i].particle)->pos,&tmp_b);
                    (entries41[i].particle)->size*=1.0f+D_0015ED60*-0.0500000119f;
                } else {
                    func_001F9A68(&tmp_a,&tmp_a,amount);
                    func_001F9A10(&(entries41[i].particle)->pos,D_L00_001E5F40+entries41[i].segment*16,&tmp_a);
                    ring=entries41[i].angle;
                    func_001F9A28(&tmp_b,(char *)D_L00_001E6020+entries41[i].segment*320+ring*16,D_L00_001E5F40+entries41[i].segment*16);
                    func_001F9A28(&tmp_c,(char *)D_L00_001E5EE0+entries41[i].segment*320+ring*16,D_L00_001E5F30+entries41[i].segment*16);
                    func_L00_0025B6B8(&motion,&tmp_b,&tmp_c,amount);
                    ring=(ring+19)%20;
                    func_001F9A28(&tmp_d,(char *)D_L00_001E6020+entries41[i].segment*320+ring*16,D_L00_001E5F40+entries41[i].segment*16);
                    func_001F9A28(&tmp_e,(char *)D_L00_001E5EE0+entries41[i].segment*320+ring*16,D_L00_001E5F30+entries41[i].segment*16);
                    func_L00_0025B6B8(&interp,&tmp_d,&tmp_e,amount);
                    func_L00_0025B6B8(&motion,&motion,&interp,entries41[i].phase);
                    entries41[i].phase+=entries41[i].phase_step;
                    if(entries41[i].phase>=1.0f) { entries41[i].phase-=1.0f; entries41[i].angle=ring; }
                    func_001F9A10(&(entries41[i].particle)->pos,&(entries41[i].particle)->pos,&motion);
                }
            }
        }
    }
}
