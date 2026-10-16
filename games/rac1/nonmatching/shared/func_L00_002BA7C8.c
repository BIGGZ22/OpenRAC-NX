/* NON_MATCHING func_L00_002BA7C8 -- src/overlays/shared/vendor_002BA7C8.c
 * Best so far: SIZE ours 5284 / retail 5292, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   - p9, run 10: COMPILE; counter rename missed the remaining arc body. Corrected in a new candidate; no usable c
 *   - p10, run 11: SIZE 5228/5292; corrected independent loop counters compile, but beam index takes s2 while the 
 *   - p11, run 12: SIZE 5212/5292; true two-track beam array shares the initial base but loop.c still gives the se
 *   - p12, run 13: SIZE 5252/5292; indexed previous-point copy reintroduces a beam high-half value over selection;
 *   - p13, run 14: SIZE 5212/5292; point-minus-one fallback removes the early beam high half and restores retail w
 *   - p14, run 15: SIZE 5212/5292; independent stack locals preserve layout and emit the same instructions as the 
 *   - p15, run 16: SIZE 5212/5292; aligned float vectors produce identical function instructions and branch graph 
 *   No source writes. Best same-size candidate p8 is BYTES 3575/5292. The p13-p15 line has a stronger matching pro
 */
#include "common.h"
typedef union { float f[4]; struct { float x,y,z,w; } v; unsigned int q __attribute__((mode(TI))); } V43;
typedef struct M43 M43;
typedef struct { char pad0[0x10]; int collision; char pad14[0x32]; short category; } Class43;
typedef struct { char pad0[4]; short kind; char pad6[10]; float height; } Aim43;
typedef struct {
    V43 position,end;
    M43 *target;
    char pad24[0x14];
    M43 *second;
    char pad3C[2];
    short count;
    union { unsigned long packed; struct { unsigned short seek0,seek1,hit0,hit1; } h; } timers;
} Weapon43;
struct M43 {
    char pad0[0x10]; V43 position;
    unsigned char state; char pad21[3]; Class43 *cls;
    char pad28[0x50]; Weapon43 *data;
    char pad7C[0x18]; int collision;
    char pad98[14]; unsigned short id;
    char padA8[0x38]; V43 forward;
};
typedef struct {
    char pad0[0x80]; V43 position;
    char pad90[8]; float yaw;
    char pad9C[0x1F4]; V43 direction;
    char pad2A0[0x3B0]; V43 row1,row2;
    char pad670[0x1A10]; M43 *moby;
} Hero43;
typedef struct { char pad0[0x140]; V43 position; char pad150[8]; float yaw; } Camera43;
typedef struct {
    V43 normal; int x,y;
    unsigned char type,mode; unsigned short id;
    float amount; int flags,pad[3];
} Damage43;
typedef struct {
    V43 target0,target1;
    Damage43 damage;
    V43 normal,direction,delta,next,step,noise;
    M43 *ignored[128];
    V43 goal;
    union { struct { V43 origin; Damage43 timed; } arc; Damage43 second; } tail;
    V43 differences[5];
} Work43;
extern char D_0013E633[] NOT_SDA;
extern Hero43 hero43 __asm__("D_0013F450");
extern Camera43 camera43 __asm__("D_L00_00166D80");
extern M43 *D_L00_001ABD80[];
extern V43 beam43[] __asm__("D_L00_001DB8C0");
extern V43 D_L00_001DB8B0[];
extern V43 noise43[] __asm__("D_L00_001DBB90");
extern float D_L00_001DBB40[], D_L00_001DBCD0[];
extern V43 raystart43[] __asm__("D_L00_001DB8A0");
extern float D_L00_00166EC0[];
extern M43 *hit43 __asm__("D_L00_00173F58");
extern V43 hitpoint43 __asm__("D_L00_00173F60");
extern V43 D_L00_001DB9F0;
extern short D_L00_00161728[4],D_L00_00161738[4],D_L00_00161740[4];
extern short arccolor43[4] __asm__("D_L00_00161730");
extern V43 arcs43[4][5] __asm__("D_L00_001DBD20");
extern V43 D_L00_001DBD10[][5], D_L00_001DBD30[][5];
extern float D_0015EE60 MACRO_ADDR;
extern float D_L00_0015F660[] MACRO_ADDR;
extern float D_L00_00161664 SDATA(D_L00_00161664);
extern float D_L00_00161668 SDATA(D_L00_00161668);
extern float D_L00_0016166C SDATA(D_L00_0016166C);
extern float D_L00_00161670 SDATA(D_L00_00161670);
extern float D_L00_00161674 SDATA(D_L00_00161674);
extern float D_L00_00161678 SDATA(D_L00_00161678);
extern float D_L00_0016167C SDATA(D_L00_0016167C);
extern float D_L00_00161680 SDATA(D_L00_00161680);
extern float D_L00_00161684 SDATA(D_L00_00161684);
extern float D_L00_00161688 SDATA(D_L00_00161688);
extern float D_L00_0016168C SDATA(D_L00_0016168C);
extern float D_L00_00161690 SDATA(D_L00_00161690);
extern float D_L00_00161694 SDATA(D_L00_00161694);
extern float D_L00_00161698 SDATA(D_L00_00161698);
extern float D_L00_0016169C SDATA(D_L00_0016169C);
extern float D_L00_001616A0 SDATA(D_L00_001616A0);
extern float D_L00_001616B4 SDATA(D_L00_001616B4);
extern float D_L00_001616B8 SDATA(D_L00_001616B8);
extern int color43 __asm__("D_L00_001616C8__gp") MACRO_ADDR;
extern float D_L00_001616F8 SDATA(D_L00_001616F8);
extern float D_L00_001616FC SDATA(D_L00_001616FC);
extern float func_001FA748(float,float),func_001FA790(float,float),func_001FA850(float,float);
extern float func_001FA888(int),func_L00_0025F368(float),func_L00_001FF860(float,float);
extern float func_001F9CE8(void*),func_001F9CB8(void*),func_001F9D10(void*,void*);
extern float func_001F9F90(float),func_001F9FA8(float);
extern float func_002140F8(float,float),func_L00_00258C80(float,float);
extern int func_001F9850(int),func_001F9938(void*),func_002140B0(int),func_L00_00258BC8(int,int),func_001FA898(float);
extern void func_001F9BC0(void*),func_001F9BF0(void*,void*,void*),func_001F9BD8(void*,void*,void*),func_001F9CA0(void*,void*,void*);
extern void func_001F9C08(void*,void*,void*,float),func_001F9C30(void*,void*,float),func_L00_001FF4B0(void*,void*,float);
extern void func_002156E0(void*,void*,void*,float);
extern Aim43 *aim43(M43*) __asm__("func_L00_0025D390");
extern int event43(int,int,M43*) __asm__("func_0022ED80");
extern void func_L00_0025A8C0(void*,void*,int,void*,float);
extern int func_L00_001F2BE8(void*,int,void*,void*,float);
extern int ray43(void*,void*,int,void*,int) __asm__("func_L00_001EFFF0");
extern void func_L00_0025AAC0(void*,void*);
extern int func_L00_0025A868(char*);
extern unsigned char *func_L00_00272F00(void*,int,float,float,int,int,int,void*,float);

/* Selects Tesla Claw targets, updates the beams and sparks, and applies damage. */
void func_L00_002BA7C8(M43 *moby) {
    Work43 w;
    int gold=(unsigned char)D_0013E633[0];
    M43 *best=0,*second=0,*candidate;
    Weapon43 *weapon=moby->data;
    Hero43 *hero;
    M43 **list;
    Aim43 *aim;
    V43 *point,*secondary,*previous,*last;
    V43 *rows,*arcpoint,*arcprev,*difference;
    float bestscore=0.0f, secondscore=0.0f;
    float length, angle, score, distance=0.0f;
    float step_length=D_L00_00161664/20.0f;
    float fi,amp,height,blend,bearing,radial,angle0,angle1,sign,size,ratio;
    int i,j,firsthalf,blocked,ignored_count,n,spin,diff;
    D_L00_0016167C=func_001FA748(D_L00_0016167C,D_L00_00161680);
    D_L00_00161684=func_001FA790(D_L00_00161684,D_L00_00161688);
    D_L00_00161698=func_001FA748(D_L00_00161698,D_L00_0016169C);
    if (!weapon->target || (gold && !weapon->second)) {
        if (weapon->second) {
            weapon->target=weapon->second;
            weapon->timers.h.seek0=weapon->timers.h.seek1;
            weapon->second=0;
            weapon->timers.h.seek1=0;
        } else {
            hero=&hero43;
            for(list=D_L00_001ABD80; (candidate=*list)!=0; list++) {
                if(candidate==weapon->target || candidate==weapon->second || !candidate || !candidate->cls || candidate->cls->category!=5 || candidate->state==0xFE || candidate->state==0xFD || !aim43(candidate)) continue;
                func_001F9BF0(&w.target0,&candidate->position,weapon);
                length=func_001F9CE8(&w.target0);
                if(D_L00_00161664<length) continue;
                angle=func_001FA850(camera43.yaw,func_L00_001FF860(candidate->position.v.x-camera43.position.v.x,candidate->position.v.y-camera43.position.v.y));
                if(D_L00_0016166C<angle) continue;
                if(D_L00_00161668<func_001FA850(func_L00_001FF860(candidate->position.v.x-hero->position.v.x,candidate->position.v.y-hero->position.v.y),hero->yaw)) continue;
                score=((D_L00_0016166C-angle)/D_L00_0016166C)*D_L00_001616FC+(D_L00_00161664-length)/D_L00_00161664;
                if(bestscore<score) {
                    secondscore=bestscore; second=best; bestscore=score; best=candidate;
                } else if(secondscore<score) {
                    secondscore=score; second=candidate;
                }
            }
        }
    }
    if(best) {
        if(!weapon->target) {
            weapon->target=best; weapon->timers.h.seek0=0;
            event43(1,0,moby);
        } else second=best;
    }
    if(second && gold) {
        distance=func_001F9D10(weapon,&best->position);
        if(distance<D_L00_00161664 && distance+func_001F9D10(&best->position,&second->position)<D_L00_00161664) {
            weapon->second=second; weapon->timers.h.seek1=0;
        }
    }
    distance=0.0f;
    if(weapon->target) {
        aim=aim43(weapon->target);
        qcopy(&w.target0,&weapon->target->position);
        w.target0.v.z+=aim->height;
        distance=func_001F9D10(weapon,&w.target0);
        step_length=distance*0.05f;
    }
    if(weapon->second) {
        aim=aim43(weapon->second);
        qcopy(&w.target1,&weapon->second->position);
        w.target1.v.z+=aim->height;
        step_length=(func_001F9D10(&w.target0,&w.target1)+distance)*0.05f;
    }
    color43=gold?0x207F20:0x7F2020;
    hero=&hero43;
    func_001F9BC0(&w.normal);
    func_L00_0025A8C0(&w.damage,moby,0x210000,&w.normal,(float)gold+2.0f);
    w.damage.type=5; w.damage.mode=1; w.damage.id=moby->id;
    weapon->count=20;
    func_L00_001F2BE8(weapon,16,hero->moby,&w.damage,0.3f);
    func_001F9BF0(&w.direction,&weapon->end,weapon);
    func_L00_001FF4B0(&w.direction,&w.direction,1.0f);
    if(weapon->target || weapon->second) {
        func_001F9CA0(&w.delta,&w.direction,&hero->direction);
        func_002156E0(&w.direction,&w.direction,&w.delta,0.17453292f);
    }
    func_001F9C30(&w.step,&w.direction,step_length);
    blocked=0; ignored_count=0;
    point=beam43;
    qcopy(point,weapon);
    secondary=point+20;
    qcopy(secondary,weapon);
    last=point; point++; secondary++;
    for(i=1;i<20;i++,point++,secondary++,last++) {
        previous=&D_L00_001DB8B0[i];
        fi=(float)i;
        amp=D_L00_00161674+(D_L00_00161678-D_L00_00161674)*(func_001FA888(i)*0.05f);
        height=amp*func_001F9FA8(func_L00_0025F368(D_L00_0016167C+fi*D_L00_00161670));
        height+=func_L00_00258C80(D_L00_001616B4,D_L00_001616B8);
        point->v.z-=D_L00_001DBB40[i];
        D_L00_001DBB40[i]=height;
        func_001F9BF0(&w.delta,point,previous);
        func_001F9C08(&w.next,&w.delta,&w.step,0.85f);
        length=func_001F9CB8(&w.next);
        if(length==0.0f) qcopy(point,last);
        else {
            func_001F9C30(&w.next,&w.next,step_length/length);
            func_001F9BD8(point,previous,&w.next);
            qcopy(&w.step,&w.next);
        }
        point->v.z+=height;
        func_001F9BF0(&w.delta,point,&D_L00_001DB8B0[i]);
        firsthalf=i<10;
        if(weapon->target) {
            if(!gold || !weapon->second) {
                func_001F9BF0(&w.goal,&w.target0,point);
                length=func_001F9CB8(&w.goal);
                if(length!=0.0f) {
                    if(i==19) { step_length=length; blend=0.5f; }
                    else if(i>=16) { step_length=length/func_001FA888(19-i); blend=0.5f; }
                    else blend=0.1f;
                    func_001F9C30(&w.goal,&w.goal,step_length/length);
                    func_001F9C08(&w.step,&w.step,&w.goal,blend);
                }
            } else {
                if(firsthalf) func_001F9BF0(&w.goal,&w.target0,point);
                else func_001F9BF0(&w.goal,&w.target1,point);
                length=func_001F9CB8(&w.goal);
                if(length!=0.0f) {
                    if(i==9 || i==19) { step_length=length; blend=0.7f; }
                    else if((unsigned)(i-5)<5 || i>=15) { step_length=length/func_001FA888(firsthalf?9-i:19-i); blend=0.7f; }
                    else blend=0.2f;
                    func_001F9C30(&w.goal,&w.goal,step_length/length);
                    func_001F9C08(&w.step,&w.step,&w.goal,blend);
                }
            }
        }
        if(firsthalf) {
            angle=func_L00_0025F368(D_L00_00161684+fi*D_L00_0016168C);
            amp=D_L00_00161690*func_001F9FA8(func_L00_0025F368(D_L00_00161698+fi*D_L00_001616A0));
            qcopy(secondary,point);
            height=func_001F9FA8(angle);
            height=amp*height+func_L00_00258C80(0.0f,0.1f);
            D_L00_001DBCD0[i]=height;
            secondary->v.z+=height;
            bearing=func_001FA748(func_L00_001FF860(w.delta.v.x,w.delta.v.y),1.57079637f);
            radial=D_L00_00161694*func_001F9F90(angle);
            w.noise.v.x=func_001F9F90(bearing)*radial;
            w.noise.v.y=func_001F9FA8(bearing)*radial;
            w.noise.v.z=0.0f;
            qcopy(&noise43[i],&w.noise);
            func_001F9BD8(secondary,secondary,&w.noise);
        } else {
            n=21-i;
            qcopy(secondary,point);
            secondary->v.z+=D_L00_001DBCD0[n];
            func_001F9BD8(secondary,secondary,&noise43[n]);
            qcopy(&w.noise,&noise43[i]);
        }
        if(i==1) {
            hero=&hero43;
            func_001F9C30(&w.tail.arc.origin,&hero->moby->forward,0.5f);
            func_001F9BD8(&w.goal,&hero->position,&w.tail.arc.origin);
            blocked=ray43(&w.goal,weapon,16,hero->moby,0);
        } else if(i==19 || !(i&1)) blocked=ray43(&raystart43[i],point,16,moby,0);
        else continue;
        func_001F9BF0(&w.normal,point,D_L00_00166EC0);
        w.normal.v.z=0.0f;
        length=func_001F9CB8(&w.normal);
        if(length==0.0f) w.normal.v.z=1.0f;
        else func_001F9C30(&w.normal,&w.normal,1.0f/length);
        w.normal.v.z=1.0f;
        w.normal.v.w=5627.925f;
        qcopy(&w.damage.normal,&w.normal);
        if(blocked) {
            candidate=hit43;
            if(candidate) {
                if(!aim43(candidate) || !candidate->cls || candidate->cls->category!=5) func_L00_0025AAC0(candidate,&w.damage);
                else {
                    blocked=0;
                    w.ignored[ignored_count++]=candidate;
                    candidate->collision=0;
                }
            }
            if(blocked) {
                qcopy(point,&hitpoint43);
                weapon->count=i;
                qcopy(&weapon->end,&hitpoint43);
                if(!hit43 || func_L00_0025A868((char*)hit43)) {
                    weapon->target=0; weapon->timers.h.seek0=0;
                }
                break;
            }
        }
    }
    for(i=0;i<ignored_count;i++) w.ignored[i]->collision=w.ignored[i]->cls->collision;
    if(!blocked) qcopy(&weapon->end,&D_L00_001DB9F0);
    if(weapon->target && !(weapon->timers.packed & 0x0000FFFF0000FFFFL)) {
        func_001F9BF0(&w.goal,&weapon->target->position,D_L00_00166EC0);
        w.goal.v.z=0.0f;
        func_L00_001FF4B0(&w.goal,&w.goal,1.0f);
        w.goal.v.z=1.0f; w.goal.v.w=5627.925f;
        func_L00_0025A8C0(&w.tail.arc.timed,moby,0x210000,&w.goal,(float)gold+2.0f);
        w.tail.arc.timed.type=5; w.tail.arc.timed.mode=3; w.tail.arc.timed.id=moby->id;
        func_L00_0025AAC0(weapon->target,&w.tail.arc.timed);
        n=5; aim=aim43(weapon->target); if(aim && aim->kind==1) n=1;
        weapon->timers.h.hit0=func_001F9850(n);
    }
    if(weapon->second && !(weapon->timers.packed & 0xFFFF0000FFFF0000L)) {
        func_001F9BF0(&w.goal,&weapon->second->position,D_L00_00166EC0);
        w.goal.v.z=0.0f;
        func_L00_001FF4B0(&w.goal,&w.goal,1.0f);
        w.goal.v.z=1.0f; w.goal.v.w=5627.925f;
        func_L00_0025A8C0(&w.tail.second,moby,0x210000,&w.goal,(float)gold+2.0f);
        w.tail.second.type=5; w.tail.second.mode=3; w.tail.second.id=moby->id;
        func_L00_0025AAC0(weapon->second,&w.tail.second);
        n=5; aim=aim43(weapon->second); if(aim && aim->kind==1) n=1;
        weapon->timers.h.hit1=func_001F9850(n);
    }
    for(i=0;i<4;i++) {
        if(i<2) { qcopy(&w.tail.arc.origin,weapon); size=0.4f; }
        else { qcopy(&w.tail.arc.origin,&weapon->end); size=0.5f; }
        if(func_001F9938(&D_L00_00161728[i])) {
            D_L00_00161728[i]=func_001F9850(D_L00_00161738[i]); arccolor43[i]=0;
            angle0=func_L00_00258C80(0.52359879f,1.04719758f);
            angle1=func_002140F8(-1.04719758f,0.17453292f);
            func_001F9C30(&w.goal,&w.direction,size);
            hero=&hero43;
            rows=&hero->row1;
            func_002156E0(&w.goal,&w.goal,rows,angle1);
            func_002156E0(&w.goal,&w.goal,rows+1,angle0);
            qcopy(&arcs43[i][0],&w.tail.arc.origin);
            func_001F9BD8(&D_L00_001DBD30[i][0],&w.tail.arc.origin,&w.goal);
            sign=1.0f;
            arcprev=&D_L00_001DBD10[i][2];
            arcpoint=&arcs43[i][2];
            for(j=2;j>=0;j--,arcpoint++,arcprev++) {
                func_001F9BF0(&w.tail.arc.timed,arcprev,D_L00_00166EC0);
                angle=func_002140F8(0.17453292f,0.95993108f)*sign;
                sign=-sign;
                func_002156E0(&w.goal,&w.goal,&w.tail.arc.timed,angle);
                func_001F9BD8(arcpoint,arcprev,&w.goal);
            }
        } else {
            arcpoint=&arcs43[i][1]; arcprev=&D_L00_001DBD10[i][1]; difference=&w.differences[1];
            for(j=3;j>=0;j--,arcpoint++,arcprev++,difference++) {
                func_001F9BF0(difference,arcpoint,arcprev);
                difference->v.x+=func_L00_00258C80(0.0f,0.1f);
                difference->v.y+=func_L00_00258C80(0.0f,0.1f);
                difference->v.z+=func_L00_00258C80(0.0f,0.1f);
            }
            qcopy(&arcs43[i][0],&w.tail.arc.origin);
            arcpoint=&arcs43[i][1]; arcprev=&D_L00_001DBD10[i][1]; difference=&w.differences[1];
            for(j=3;j>=0;j--,arcpoint++,arcprev++,difference++) func_001F9BD8(arcpoint,arcprev,difference);
            diff=D_L00_00161740[i]; n=D_L00_00161728[i];
            if(diff<n) diff=n-diff; else diff=diff-n;
            ratio=func_001FA888(diff)/func_001FA888(func_001F9850(15));
            arccolor43[i]=func_001FA898((1.0f-ratio)*32.0f);
        }
    }
    rows=&hero43.row1;
    angle0=func_L00_00258C80(0.52359879f,1.04719758f);
    angle1=func_002140F8(-0.52359879f,1.04719758f);
    func_001F9C30(&w.goal,&w.direction,D_L00_001616F8*D_0015EE60);
    hero=&hero43;
    func_002156E0(&w.goal,&w.goal,rows,angle1);
    func_002156E0(&w.goal,&w.goal,rows+1,angle0);
    spin=func_L00_00258BC8(1,8); if(func_002140B0(2)) spin=-spin;
    size=func_002140F8(0.2f,0.6f);
    func_L00_00272F00(weapon,func_001F9850(15),size*0.3f,size,color43|0x7F000000,0,spin,&w.goal,0.0f);
    spin=-spin;
    func_L00_00272F00(weapon,func_001F9850(15),size*0.15f,size*0.5f,0x307F7F7F,0,spin,&w.goal,0.0f);
    spin=func_L00_00258BC8(1,8); if(func_002140B0(2)) spin=-spin;
    size=func_002140F8(0.15f,3.0f);
    func_L00_00272F00(&weapon->end,func_001F9850(12),size*0.1f,size,color43|0x7F000000,0,spin,D_L00_0015F660,0.0f);
}
