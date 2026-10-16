/* NON_MATCHING func_L00_002DF410 -- src/overlays/shared/vendor_002D9438.c
 * Best so far: SIZE ours 4216 / retail 4220, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - Run 7: p6.c SIZE ours 4160 / retail 4220
 *   - Run 8: p7.c SIZE ours 4200 / retail 4220
 *   - Run 9: p8.c SIZE ours 4208 / retail 4220
 *   - Run 10: p9.c SIZE ours 4208 / retail 4220
 *   - Run 11: p10.c SIZE ours 4216 / retail 4220
 *   Further trials: p10 captures the old vertex index before increment (4216 bytes, still reverses the counter and
 *   Final: stop after 18/30 trials because p15, p16 and p17 compile to identical function instructions and branch 
 *   p13's aggregate initializer emitted memset calls and is unsuitable. p14's returning-vector constructor reserve
 */
#include "common.h"
typedef unsigned int Q41 __attribute__((mode(TI)));
typedef union { struct { float x,y,z,w; } f; Q41 q; } V41 __attribute__((aligned(16)));
typedef struct {
    V41 step, up, orient[3], ray_end;
    V41 start,end,axis,tmp_a,tmp_b,cross_b,cross_a,basis,tmp_c,motion;
    V41 tmp_d,tmp_e,interp,cross_c,basis_c,tmp_f;
} Work41;
typedef struct {
    u8 pad0[0x34]; u16 flags; u8 pad36[0x8A]; V41 basis[3];
} M41;
typedef struct {
    u8 pad0[0x640]; V41 matrix[3]; u8 pad670[0x1A10];
    M41 *moby; int camera_mode; u8 pad2088[0x240]; s16 special;
} Hero41;
typedef struct Particle41 {
    u8 pad0[0xA]; s16 life; float size; V41 pos;
    float unknown20; int mode; u8 pad28[2]; u8 r,g; u8 pad2C[4];
    V41 velocity; float acceleration;
} Particle41;
typedef struct {
    s16 segment,angle; float distance,phase,phase_step,speed; Particle41 *particle;
} Entry41;
extern char D_0013E633[] NOT_SDA;
extern char D_L00_001670D0[];
extern float D_L00_00161C54 MACRO_ADDR;
extern char D_L00_001E6290[];
extern char D_L00_001E6280[];
extern char D_L00_00173F60[];
extern short D_L00_001E6358[];
extern V41 D_L00_00161C30 MACRO_ADDR;
extern V41 D_L00_00161C40 MACRO_ADDR;
extern float D_L00_00161C50 MACRO_ADDR;
extern int tick41 SDATA(D_L00_00161C24);
extern float wave41 SDATA(D_L00_00161C28);
extern float motion_scale41 SDATA(D_L00_00161C54);
extern V41 D_L00_001E4E90[];
extern V41 D_L00_001E6370[];
extern V41 D_L00_001E6230[];
extern float D_L00_001E6330[];
extern char D_L00_001E4FE4[],D_L00_001E4FD4[],D_L00_001E4FE0[];
extern Entry41 entries41[] __asm__("D_L00_001E4FD0");
extern float D_0015EE60 MACRO_ADDR;
extern void func_L00_002DF168(void *,void *);
extern void func_001F9C30(void *,void *,float);
extern void func_L00_001FF4B0(void *,void *,float);
extern void func_001F9BD8(void *,void *,void *);
extern int func_L00_001EFFF0(void *,void *,int,int,int);
extern float func_001F9D48(void *,void *);
extern int func_001FA898(float);
extern void func_001F9EC0(void *,void *,void *);
extern int func_001F9850(int);
extern float func_001FA888(int);
extern float func_002140F8(float,float);
extern float func_001FA748(float,float);
extern float func_001FA790(float,float);
extern void func_002156E0(void *,void *,float,void *);
extern void func_001F9CA0(void *,void *,void *);
extern void func_001F9BF0(void *,void *,void *);
extern float func_001F9C78(void *,void *);
extern float func_001F9CB8(void *);
extern float func_001F9FC0(float);
extern float func_L00_001EB6A8(void *,float,float,float,float,float);
extern float func_001F9B88(float);
extern void func_001F9C08(void *,void *,void *,float);
extern float func_001F9FA8(float);
extern void func_L00_002DED98(void);
extern void func_001F49B0(void (*)(void),int);
extern int func_002140B0(int);
extern Particle41 *new_particle41(void *,int,void *,int,float,float,float,float) __asm__("func_L00_0026DEA0");
extern void func_001F9BC0(void *);
extern void func_L00_002688A8(void *);
extern void func_L00_0025C710(void *,void *,void *,float);

static __inline__ void clear_vectors41(V41 *dst_a,V41 *dst_b) {
    register V41 a,b;
    a.q=0;
    b.q=0;
    dst_a->q=a.q;
    dst_b->q=b.q;
}

/* Form the Suck Cannon's bending suction tube and update its particles. */
void func_L00_002DF410(void *moby,void *position) {
    V41 step,up,orient[3],ray_end,start,end,axis,tmp_a,tmp_b,
        cross_b,cross_a,basis,tmp_c,motion,tmp_d,tmp_e,interp,cross_c,basis_c,tmp_f;
    V41 *orientation;
    int i,j,n,count,sparks,segment,ring,next_ring;
    float t,angle,turn,delta,dot,length,radius,phase,amount,speed;
    Particle41 *p;
    V41 *anchor,*previous,*source;
    Entry41 *entry;


    func_L00_002DF168(moby,position);
    if((((Hero41 *)(D_0013E633+0xE1D))->camera_mode==30 || ((Hero41 *)(D_0013E633+0xE1D))->camera_mode==1) && (((Hero41 *)(D_0013E633+0xE1D))->moby->flags&1)) {
        qcopy(&orient[0],D_L00_001670D0);
        qcopy(&orient[1],D_L00_001670D0+16);
        qcopy(&orient[2],D_L00_001670D0+32);
        orientation=&orient[0];
    } else if(((Hero41 *)(D_0013E633+0xE1D))->special==1) {
        func_001F9C30(&orient[0],&((M41 *)moby)->basis[1],-1.0f);
        { M41 *m=moby;
        qcopy_nc(&orient[1],&m->basis[0]);
        qcopy_nc(&orient[2],&m->basis[2]); }
        orientation=&orient[0];
    } else {
        qcopy(&orient[0],&((Hero41 *)(D_0013E633+0xE1D))->matrix[0]);
        qcopy(&orient[1],&((Hero41 *)(D_0013E633+0xE1D))->matrix[1]);
        qcopy(&orient[2],&((Hero41 *)(D_0013E633+0xE1D))->matrix[2]);
        orientation=&orient[0];
    }
    func_L00_001FF4B0(&step,orientation,D_L00_00161C54);
    func_L00_001FF4B0(&up,&orient[2],1.0f);
    qcopy(D_L00_001E6290,position);
    func_L00_001FF4B0(&ray_end,orientation,D_L00_00161C54*10.0f);
    func_001F9BD8(&ray_end,&ray_end,position);
    if(func_L00_001EFFF0(position,&ray_end,20,(int)((Hero41 *)(D_0013E633+0xE1D))->moby,0)) {
        int n,i;
        n=func_001FA898(func_001F9D48(D_L00_00173F60,position)/D_L00_00161C54);
        for(i=n;i<10;i++) {
            if(D_L00_001E6358[i]>64)D_L00_001E6358[i]=64;
            D_L00_001E6358[i]=(u16)D_L00_001E6358[i]-4;
            if(D_L00_001E6358[i]<0)D_L00_001E6358[i]=0;
        }
        for(i=n-1;i>=0;i--)D_L00_001E6358[i]=D_L00_001E6358[i+1]+32;
    } else {
        int i;
        D_L00_001E6358[0]=(u16)D_L00_001E6358[0]+4;
        for(i=1;i<10;i++)D_L00_001E6358[i]=D_L00_001E6358[i-1]-32;
    }
    func_001F9EC0(&start,&D_L00_00161C30,orientation);
    func_001F9BD8(&start,&start,&step);
    func_001F9EC0(&end,&D_L00_00161C40,orientation);
    func_001F9BD8(&end,&end,&step);
    t=(float)tick41/func_001FA888(func_001F9850(60));
    tick41++;
    step.f.x=start.f.x+(end.f.x-start.f.x)*t;
    step.f.y=start.f.y+(end.f.y-start.f.y)*t;
    step.f.z=start.f.z+(end.f.z-start.f.z)*t;
    if(tick41>=func_001F9850(60)) {
        qcopy(&D_L00_00161C30,&D_L00_00161C40);
        func_L00_001FF4B0(&axis,orientation,D_L00_00161C54);
        clear_vectors41(&tmp_a,&tmp_b);
        tmp_a.f.z=0.1f; tmp_a.f.w=1.0f;
        tmp_b.f.x=1.0f; tmp_b.f.w=1.0f;
        angle=func_001FA748(D_L00_00161C50,func_002140F8(45.0f,315.0f)*0.0174532924f);
        func_002156E0(&D_L00_00161C40,&tmp_a,angle,&tmp_b);
        D_L00_00161C50=angle;
        tick41=0;
    }
    wave41=func_001FA748(wave41,0.1f);
    qcopy(&axis,&orient[2]);
    qcopy(&basis,&axis);
    func_001F9CA0(&cross_a,&step,&basis);
    func_L00_001FF4B0(&cross_a,&cross_a,1.0f);
    func_001F9CA0(&cross_b,&basis,&cross_a);
    { int i;
    for(i=19;i>=0;i--) {
        func_001F9C30(&tmp_a,&D_L00_001E4E90[19-i],0.1f);
        func_001F9EC0(&tmp_a,&tmp_a,&cross_b);
        func_001F9BD8(&D_L00_001E6370[19-i],D_L00_001E6290,&tmp_a);
    }
    }
    { int i;
    for(i=1;i<10;i++) {
        anchor=(V41 *)(D_L00_001E6290+i*16);
        previous=(V41 *)(D_L00_001E6280+i*16);
        func_001F9BF0(&tmp_a,anchor,previous);
        qcopy(&tmp_b,&step);
        func_001F9CA0(&tmp_c,&tmp_b,&tmp_a);
        dot=func_001F9C78(&tmp_a,&tmp_b);
        angle=1.5707964f-func_001F9FC0(dot/(func_001F9CB8(&tmp_a)*D_L00_00161C54));
        turn=func_L00_001EB6A8(&D_L00_001E6330[i],0.0f,angle,0.06f,0.2f,0.0f);
        angle=func_001FA790(angle,turn);
        if(func_001F9B88(angle)>0.31415927f)turn=func_001FA748(turn,func_001FA790(angle,0.31415927f));
        func_002156E0(&motion,&tmp_a,turn,&tmp_c);
        func_L00_001FF4B0(&tmp_d,&motion,motion_scale41);
        func_001F9C08(&motion,&motion,&tmp_d,1.0f);
        { float len=func_001F9CB8(&motion);
        if(len>2.0f)func_001F9C30(&motion,&motion,2.0f/len); }
        amount=(float)i;
        if(i>=6)func_001F9C08(&motion,&motion,&tmp_a,amount/func_001FA888(10)+0.0f);
        func_001F9BD8(anchor,previous,&motion);
        qcopy(&tmp_e,&step);
        qcopy(&basis_c,&axis);
        func_L00_001FF4B0(&interp,&tmp_e,1.0f);
        func_001F9CA0(&cross_c,&interp,&basis_c);
        func_L00_001FF4B0(&cross_c,&cross_c,1.0f);
        func_001F9CA0(&basis_c,&cross_c,&interp);
        qcopy(&axis,&basis_c);
        radius=amount*0.3f;
        phase=amount*0.7f;
        { int j;
        for(j=0;j<20;j++) {
            angle=radius+0.1f;
            angle+=func_001F9FA8(func_001FA748(wave41+phase,0.0f))*0.1f;
            func_001F9C30(&tmp_f,(char *)D_L00_001E4E90+j*16,angle);
            func_001F9EC0(&tmp_f,&tmp_f,&interp);
            func_001F9BD8((char *)D_L00_001E6370+i*320+j*16,D_L00_001E6290+i*16,&tmp_f);
        }
        }
        func_L00_001FF4B0(&step,&motion,D_L00_00161C54);
    }
    }
    func_001F49B0(func_L00_002DED98,(int)moby);
    count=0; sparks=0;
    for(i=0;i<200;i++) {
        if(!entries41[i].particle && count<5) {
            for(segment=9;segment>=0 && D_L00_001E6358[segment]<64;segment--);
            if(segment>0) {
                if(segment>=2 && sparks<2) {
                    ring=func_002140B0(20);
                    amount=func_002140F8(0.2f,1.0f);
                    source=&D_L00_001E6370[ring];
                    func_001F9C08(&tmp_a,source,source+segment*20,amount);
                    func_001F9C08(&tmp_c,D_L00_001E6290,D_L00_001E6290+segment*16,amount);
                    speed=func_002140F8(60000.0f,180000.0f);
                    p=new_particle41(&tmp_a,func_002140B0(5)+1,&tmp_b,0x404040,0.0f,1.0f,1.03f,speed);
                    if(p) {
                        func_001F9BF0(&motion,&tmp_a,&tmp_c);
                        sparks++;
                        func_L00_001FF4B0(&p->velocity,&motion,0.01f);
                        p->velocity.f.z+=0.02f;
                    }
                }
                qcopy(&tmp_a,D_L00_001E6290+segment*16);
                func_001F9BC0(&tmp_b);
                speed=func_002140F8(60000.0f,180000.0f);
                p=new_particle41(&tmp_a,6,&tmp_b,0x7F404040,0.0f,1.0f,1.0f,speed);
                entries41[i].particle=p;
                if(p) { p->life=200; p->mode=2; count++; p->r=127; p->g=200; }
                entry=&entries41[i];
                entries41[i].segment=segment;
                entries41[i].distance=0;
                entries41[i].phase=0;
                entries41[i].angle=func_002140B0(20);
                entries41[i].phase_step=func_002140F8(0.1f,0.8f);
                entries41[i].speed=func_002140F8(0.1f,0.4f);
            }
        }
        if(entries41[i].particle) {
            entry=&entries41[i];
            func_001F9BF0(&tmp_a,D_L00_001E6280+entries41[i].segment*16,D_L00_001E6290+entries41[i].segment*16);
            length=func_001F9CB8(&tmp_a);
            entries41[i].distance+=entries41[i].speed;
            (entries41[i].particle)->life=200;
            if(length<entries41[i].distance) {
                entries41[i].segment=(u16)entries41[i].segment-1;
                if(entries41[i].segment<=0) { func_L00_002688A8(entries41[i].particle); entries41[i].particle=0; }
                else {
                    entries41[i].distance-=length;
                    func_001F9BF0(&tmp_a,D_L00_001E6280+entries41[i].segment*16,D_L00_001E6290+entries41[i].segment*16);
                    length=func_001F9CB8(&tmp_a);
                }
            }
                if(entries41[i].particle) {
                entry=&entries41[i];
                amount=entries41[i].distance/length;
                if(entries41[i].segment==2)(entries41[i].particle)->life=100;
                else if(entries41[i].segment<2) {
                    (entries41[i].particle)->life=func_001FA898((1.0f-amount)*100.0f);
                    if((entries41[i].particle)->life<61)(entries41[i].particle)->life=60;
                }
                entry=&entries41[i];
                if(entries41[i].segment==1) {
                    func_001F9BF0(&tmp_b,position,&(entries41[i].particle)->pos);
                    func_L00_001FF4B0(&tmp_b,&tmp_b,entries41[i].speed);
                    func_001F9BD8(&(entries41[i].particle)->pos,&(entries41[i].particle)->pos,&tmp_b);
                    (entries41[i].particle)->size*=1.0f+D_0015EE60*-0.0500000119f;
                } else {
                    func_001F9C30(&tmp_a,&tmp_a,amount);
                    func_001F9BD8(&(entries41[i].particle)->pos,D_L00_001E6290+entries41[i].segment*16,&tmp_a);
                    ring=entries41[i].angle;
                    func_001F9BF0(&tmp_b,(char *)D_L00_001E6370+entries41[i].segment*320+ring*16,D_L00_001E6290+entries41[i].segment*16);
                    func_001F9BF0(&tmp_c,(char *)D_L00_001E6230+entries41[i].segment*320+ring*16,D_L00_001E6280+entries41[i].segment*16);
                    func_L00_0025C710(&motion,&tmp_b,&tmp_c,amount);
                    ring=(ring+19)%20;
                    func_001F9BF0(&tmp_d,(char *)D_L00_001E6370+entries41[i].segment*320+ring*16,D_L00_001E6290+entries41[i].segment*16);
                    func_001F9BF0(&tmp_e,(char *)D_L00_001E6230+entries41[i].segment*320+ring*16,D_L00_001E6280+entries41[i].segment*16);
                    func_L00_0025C710(&interp,&tmp_d,&tmp_e,amount);
                    func_L00_0025C710(&motion,&motion,&interp,entries41[i].phase);
                    entries41[i].phase+=entries41[i].phase_step;
                    if(entries41[i].phase>=1.0f) { entries41[i].phase-=1.0f; entries41[i].angle=ring; }
                    func_001F9BD8(&(entries41[i].particle)->pos,&(entries41[i].particle)->pos,&motion);
                }
            }
        }
    }
}
