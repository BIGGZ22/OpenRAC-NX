/* Unmatched plain-C reconstruction. 5620 bytes versus 5868; -G0.
 * instruction at +0x4 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
typedef struct { Vec direction; int owner,flags; unsigned char type,active; unsigned short class_id; float material; int enabled; } HitPacket;
extern unsigned char D_0013957A;
extern char *D_0018C0B0,*D_L00_001FACE0[],*D_L00_001BF218;
extern char D_L00_00262570[];
extern Vec D_00189EA0,D_0018A0E0,D_0018A560,D_0018A570,D_L00_001A8F20,D_L00_001BF220,D_L00_001B8DC0;
extern float D_L00_001B8DD8,D_00189EB8;
extern int D_L00_001A8FF0,D_L00_001B2748;
extern short D_L00_001B2750[],D_L00_001B2758[],D_L00_001B2760[],D_L00_001B2768[];
extern Vec D_L00_00239950[20],D_L00_00239A90[20],D_L00_00239C20[20],D_L00_00239DB0[20];
extern float D_L00_00239BD0[20],D_L00_00239D60[20];
extern float D_L00_001AF134 __attribute__((sda)),D_L00_001AF138 __attribute__((sda)),D_L00_001AF13C __attribute__((sda)),D_L00_001AF140 __attribute__((sda)),D_L00_001AF144 __attribute__((sda)),D_L00_001AF148 __attribute__((sda)),D_L00_001AF14C __attribute__((sda)),D_L00_001AF150 __attribute__((sda)),D_L00_001AF154 __attribute__((sda)),D_L00_001AF158 __attribute__((sda)),D_L00_001AF15C __attribute__((sda)),D_L00_001AF160 __attribute__((sda)),D_L00_001AF164 __attribute__((sda)),D_L00_001AF168 __attribute__((sda)),D_L00_001AF16C __attribute__((sda)),D_L00_001AF170 __attribute__((sda)),D_L00_001AF184 __attribute__((sda)),D_L00_001AF188 __attribute__((sda)),D_L00_001AF1C8 __attribute__((sda)),D_L00_001AF1CC __attribute__((sda));
extern unsigned int D_L00_001AF198 __attribute__((sda));
extern float func_00283B98(float,float),func_00283BE0(float,float),func_00283C80(float,float),func_00283248(float,float),func_002831B0(float),func_00283180(float),func_00283198(float),func_002ABB18(float),func_00283CE0(int),func_002A78D8(float,float),func_002A7878(float,float);
extern int func_00283CF0(float),func_00282948(void *),func_00282978(void *),func_002A77E0(int),func_002A7820(int,int),func_002761F8(void *,void *,int,void *,void *);
extern float func_00282E20(void *),func_00282DF0(void *),func_00282DB0(void *,void *),func_00282E48(void *,void *);
extern char *func_002AB1F0(void *);
extern void func_00282CF0(void *,void *,void *),func_00282CC0(void *,void *,void *),func_00282D30(void *,void *,float),func_00282F20(void *,void *,float),func_00282D08(void *,void *,void *,float),func_00282DD8(void *,void *,void *),func_00282C88(void *),func_00300190(void *,void *),func_002ACEC0(void *,void *,void *,float);
extern void func_002A8C00(void *,void *,int,void *,float),func_00277F48(void *,int,void *,void *,float),func_002A8E20(void *,void *),func_002C2210(void *,int,int,int,int,void *,int,float,float,float);
#define I(p,o) (*(int *)((p)+(o)))
#define F(p,o) (*(float *)((p)+(o)))
#define H(p,o) (*(short *)((p)+(o)))
#define P(p,o) (*(char **)((p)+(o)))
void func_L00_00393148(char *obj) {
    struct { Vec first,second,temp; HitPacket collision; Vec direction,delta,blend,step,offset;
        char *disabled[128]; Vec toward,scratch; HitPacket hit1,hit2; Vec segments[4]; } frame;
    char *ctx=P(obj,0x68),*candidate,*definition,*best=0,*runner=0,*hit,**list;
    Vec *point,*other,*branch;
    int upgraded,blocked=0,disabled=0,i,j,k,rotation,firsthalf;
    float bestscore=0,secondscore=0,reach=D_L00_001AF134,distance,score,heading,playerangle,dot,length,scale=0,amplitude,phase,wave,noise,blend,angle,sideangle,size;
    upgraded=D_L00_00262570[D_0013957A*0xE0+4]!=0;
    D_L00_001AF14C=func_00283B98(D_L00_001AF14C,D_L00_001AF150);
    D_L00_001AF154=func_00283BE0(D_L00_001AF154,D_L00_001AF158);
    D_L00_001AF168=func_00283B98(D_L00_001AF168,D_L00_001AF16C);
    if(!P(ctx,0x20) || (upgraded && !P(ctx,0x38))) {
        if(!P(ctx,0x38)) {
            for(list=D_L00_001FACE0;(candidate=*list)!=0;++list) {
                if(candidate==P(ctx,0x20) || candidate==P(ctx,0x38) || !candidate || !P(candidate,0x24) || H(P(candidate,0x24),0x46)!=5) continue;
                if((signed char)candidate[0x20]==-2 || (signed char)candidate[0x20]==-3) continue;
                definition=func_002AB1F0(candidate); if(!definition) continue;
                func_00282CF0(&frame.first,candidate+0x10,ctx);
                if(!D_L00_001A8FF0) distance=func_00282E20(&frame.first);
                else {
                    dot=func_00282DB0(&frame.first,D_0018C0B0+0xE0);
                    func_00282D30(&frame.second,D_0018C0B0+0xE0,dot); func_00282CF0(&frame.temp,&frame.first,&frame.second); distance=func_00282DF0(&frame.temp);
                }
                if(distance>D_L00_001AF134) continue;
                heading=func_00283C80(D_L00_001B8DD8,func_00283248(F(candidate,0x10)-D_L00_001B8DC0.f[0],F(candidate,0x14)-D_L00_001B8DC0.f[1]));
                if(heading>D_L00_001AF13C) continue;
                playerangle=0;
                if(!D_L00_001A8FF0) playerangle=func_00283C80(func_00283248(F(candidate,0x10)-D_00189EA0.f[0],F(candidate,0x14)-D_00189EA0.f[1]),D_00189EB8);
                else {
                    func_00282CF0(&frame.second,candidate+0x10,&D_00189EA0);
                    dot=func_00282DB0(&frame.second,D_0018C0B0+0xE0); func_00282D30(&frame.temp,D_0018C0B0+0xE0,dot); func_00282CF0(&frame.second,&frame.second,&frame.temp);
                    dot=func_00282DB0(&frame.second,D_0018C0B0+0xC0); length=func_00282DF0(&frame.second); if(length==0) continue;
                    playerangle=1.5707964f-func_002831B0(dot/length);
                }
                if(playerangle>D_L00_001AF138) continue;
                score=((D_L00_001AF13C-heading)/D_L00_001AF13C)*D_L00_001AF1CC+(D_L00_001AF134-distance)/D_L00_001AF134;
                if(bestscore<score) { runner=best; secondscore=bestscore; bestscore=score; best=candidate; }
                else if(secondscore<score) { secondscore=score; runner=candidate; }
            }
        } else { P(ctx,0x20)=P(ctx,0x38); H(ctx,0x40)=H(ctx,0x42); P(ctx,0x38)=0; H(ctx,0x42)=0; }
    }
    if(best) {
        if(!P(ctx,0x20)) { H(ctx,0x40)=0; P(ctx,0x20)=best; if(func_00282948(&D_L00_001B2748)) D_L00_001B2748=10; }
        else runner=best;
    }
    if(runner && upgraded) {
        distance=func_00282E48(ctx,best+0x10);
        if(distance<D_L00_001AF134 && distance+func_00282E48(best+0x10,runner+0x10)<D_L00_001AF134) { H(ctx,0x42)=0; P(ctx,0x38)=runner; }
    }
    distance=0;
    if(P(ctx,0x20)) {
        definition=func_002AB1F0(P(ctx,0x20)); candidate=P(ctx,0x20); frame.first.q=*(Quad *)(candidate+0x10);
        func_00282D30(&frame.second,candidate+0xE0,F(definition,0x10)); func_00282CC0(&frame.first,&frame.first,&frame.second);
        reach=distance=func_00282E48(ctx,&frame.first);
    }
    if(P(ctx,0x38)) {
        definition=func_002AB1F0(P(ctx,0x38)); candidate=P(ctx,0x38); frame.second.q=*(Quad *)(candidate+0x10);
        func_00282D30(&frame.temp,candidate+0xE0,F(definition,0x10)); func_00282CC0(&frame.second,&frame.second,&frame.temp);
        reach=func_00282E48(&frame.first,&frame.second)+distance;
    }
    reach*=0.05f;
    if(upgraded) D_L00_001AF198=0x207F20; else D_L00_001AF198=0x7F2020;
    func_00282C88(&frame.temp);
    func_002A8C00(&frame.collision,obj,0x210000,&frame.temp,F(D_L00_00262570+D_0013957A*0xE0,0x70));
    frame.collision.class_id=*(unsigned short *)(obj+0xAA); frame.collision.active=1; frame.collision.type=5; H(ctx,0x3E)=20;
    func_00277F48(ctx,0x10,D_0018C0B0,&frame.collision,0.3f);
    func_00282CF0(&frame.direction,ctx+0x10,ctx); func_00282F20(&frame.direction,&frame.direction,1.0f);
    if(P(ctx,0x20) || P(ctx,0x38)) { func_00282DD8(&frame.delta,&frame.direction,&D_0018A0E0); func_002ACEC0(&frame.direction,&frame.direction,&frame.delta,0.17453295f); }
    func_00282D30(&frame.step,&frame.direction,reach);
    D_L00_00239950[0].q=*(Quad *)ctx; D_L00_00239A90[0].q=*(Quad *)ctx;
    for(i=1;i<=19;++i) {
        point=&D_L00_00239950[i]; other=&D_L00_00239A90[i];
        amplitude=D_L00_001AF144+(D_L00_001AF148-D_L00_001AF144)*func_00283CE0(i)*0.05f;
        phase=func_002ABB18(D_L00_001AF14C+(float)i*D_L00_001AF140);
        wave=func_00283198(phase); noise=func_002A78D8(D_L00_001AF184,D_L00_001AF188); noise=amplitude*wave+noise;
        point->f[2]-=D_L00_00239BD0[i]; D_L00_00239BD0[i]=noise;
        func_00300190(point,point);
        func_00282CF0(&frame.delta,point,&D_L00_00239950[i-1]); func_00282D08(&frame.blend,&frame.delta,&frame.step,0.85f);
        length=func_00282DF0(&frame.blend);
        if(length==0) point->q=D_L00_00239950[i-1].q;
        else { func_00282D30(&frame.blend,&frame.blend,reach/length); func_00282CC0(point,&D_L00_00239950[i-1],&frame.blend); frame.step.q=frame.blend.q; }
        point->f[2]+=noise; func_00282CF0(&frame.delta,point,&D_L00_00239950[i-1]);
        if(P(ctx,0x20)) {
            if(!upgraded || !P(ctx,0x38)) {
                func_00282CF0(&frame.toward,&frame.first,point); length=func_00282DF0(&frame.toward);
                if(length!=0) {
                    if(i==19) { blend=0.5f; reach=length; } else if(i<16) blend=0.1f; else { blend=0.5f; reach=length/func_00283CE0(19-i); }
                    func_00282D30(&frame.toward,&frame.toward,reach/length); func_00282D08(&frame.step,&frame.step,&frame.toward,blend);
                }
            } else {
                if(i<10) func_00282CF0(&frame.toward,&frame.first,point); else func_00282CF0(&frame.toward,&frame.second,point);
                length=func_00282DF0(&frame.toward);
                if(length!=0) {
                    if(i==9 || i==19) { blend=0.7f; reach=length; }
                    else if((unsigned int)i-5<5 || i>14) { blend=0.7f; reach=length/func_00283CE0(i<10?9-i:19-i); } else blend=0.2f;
                    func_00282D30(&frame.toward,&frame.toward,reach/length); func_00282D08(&frame.step,&frame.step,&frame.toward,blend);
                }
            }
        }
        if(i<10) {
            phase=func_002ABB18(D_L00_001AF154+(float)i*D_L00_001AF15C);
            angle=func_002ABB18(D_L00_001AF168+(float)i*D_L00_001AF170);
            amplitude=D_L00_001AF160*func_00283198(angle); other->q=point->q;
            wave=func_00283198(phase); noise=func_002A78D8(0.0f,0.1f); noise=amplitude*wave+noise;
            D_L00_00239D60[i]=noise; other->f[2]+=noise;
            angle=func_00283B98(func_00283248(frame.delta.f[0],frame.delta.f[1]),1.5707965f);
            amplitude=D_L00_001AF164*func_00283180(phase);
            frame.offset.f[0]=func_00283180(angle)*amplitude; frame.offset.f[1]=func_00283198(angle)*amplitude;
            D_L00_00239C20[i].q=frame.offset.q; frame.offset.f[2]=0; func_00282CC0(other,other,&frame.offset);
        } else { other->q=point->q; other->f[2]+=D_L00_00239D60[21-i]; func_00282CC0(other,other,&D_L00_00239C20[21-i]); frame.offset.q=D_L00_00239C20[i].q; }
        if(i>1 && (i==19 || !(i&1))) {
            func_00282CF0(&frame.temp,point,&D_L00_001B8DC0);
            if(!D_L00_001A8FF0) frame.temp.f[2]=0; else { dot=func_00282DB0(&frame.temp,D_0018C0B0+0xE0); func_00282D30(&frame.toward,D_0018C0B0+0xE0,dot); func_00282CF0(&frame.temp,&frame.temp,&frame.toward); }
            length=func_00282DF0(&frame.temp); if(length==0) frame.temp.f[2]=1.0f; else func_00282D30(&frame.temp,&frame.temp,1.0f/length);
            frame.temp.f[3]=5627.925f; frame.collision.direction.q=frame.temp.q;
            blocked=func_002761F8(&D_L00_00239950[i-2],point,0x10,obj,0);
            if(blocked) {
                hit=D_L00_001BF218;
                if(hit) { definition=func_002AB1F0(hit); if(!definition || !P(hit,0x24) || H(P(hit,0x24),0x46)!=5) func_002A8E20(hit,&frame.collision); else { blocked=0; frame.disabled[disabled++]=hit; I(hit,0x98)=0; } }
                if(blocked) { point->q=D_L00_001BF220.q; H(ctx,0x3E)=i; *(Quad *)(ctx+0x10)=D_L00_001BF220.q; if(!D_L00_001BF218) { P(ctx,0x20)=0; H(ctx,0x40)=0; } break; }
            }
        }
    }
    for(i=0;i<disabled;++i) { hit=frame.disabled[i]; I(hit,0x98)=I(P(hit,0x24),0x10); }
    if(!blocked) *(Quad *)(ctx+0x10)=D_L00_00239950[19].q;
    if(P(ctx,0x20) && H(ctx,0x40)==0 && H(ctx,0x44)==0) {
        func_00282CF0(&frame.toward,P(ctx,0x20)+0x10,&D_L00_001B8DC0);
        if(!D_L00_001A8FF0) frame.toward.f[2]=0; else { dot=func_00282DB0(&frame.toward,D_0018C0B0+0xE0); func_00282D30(&frame.scratch,D_0018C0B0+0xE0,dot); func_00282CF0(&frame.toward,&frame.toward,&frame.scratch); }
        func_00282F20(&frame.toward,&frame.toward,1.0f); frame.toward.f[3]=5627.925f;
        func_002A8C00(&frame.hit1,obj,0x210000,&frame.toward,F(D_L00_00262570+D_0013957A*0xE0,0x70));
        frame.hit1.class_id=*(unsigned short *)(obj+0xAA); frame.hit1.active=3; frame.hit1.type=5;
        func_002A8E20(P(ctx,0x20),&frame.hit1); definition=func_002AB1F0(P(ctx,0x20)); H(ctx,0x44)=definition && H(definition,4)==1?1:5;
    }
    if(P(ctx,0x38) && H(ctx,0x42)==0 && H(ctx,0x46)==0) {
        func_00282CF0(&frame.toward,P(ctx,0x38)+0x10,&D_L00_001B8DC0);
        if(!D_L00_001A8FF0) frame.toward.f[2]=0; else { dot=func_00282DB0(&frame.toward,D_0018C0B0+0xE0); func_00282D30(&frame.scratch,D_0018C0B0+0xE0,dot); func_00282CF0(&frame.toward,&frame.toward,&frame.scratch); }
        func_00282F20(&frame.toward,&frame.toward,1.0f); frame.toward.f[3]=5627.925f;
        func_002A8C00(&frame.hit2,obj,0x210000,&frame.toward,F(D_L00_00262570+D_0013957A*0xE0,0x70));
        frame.hit2.class_id=*(unsigned short *)(obj+0xAA); frame.hit2.active=3; frame.hit2.type=5;
        func_002A8E20(P(ctx,0x38),&frame.hit2); definition=func_002AB1F0(P(ctx,0x38)); H(ctx,0x46)=definition && H(definition,4)==1?1:5;
    }
    for(i=0;i<4;++i) {
        if(i<2) { frame.scratch.q=*(Quad *)ctx; scale=0.4f; } else { frame.scratch.q=*(Quad *)(ctx+0x10); scale=0.5f; }
        branch=&D_L00_00239DB0[i*5];
        if(!func_00282978(&D_L00_001B2750[i])) {
            for(j=0;j<4;++j) {
                func_00282CF0(&frame.segments[j],&branch[j+1],&branch[j]);
                frame.segments[j].f[0]+=func_002A78D8(0.0f,0.1f); frame.segments[j].f[1]+=func_002A78D8(0.0f,0.1f); frame.segments[j].f[2]+=func_002A78D8(0.0f,0.1f);
            }
            branch[0].q=frame.scratch.q;
            for(j=0;j<4;++j) func_00282CC0(&branch[j+1],&branch[j],&frame.segments[j]);
            k=D_L00_001B2750[i]-D_L00_001B2768[i]; if(k<0) k=-k;
            length=func_00283CE0(k); amplitude=func_00283CE0(15); D_L00_001B2758[i]=func_00283CF0((1.0f-length/amplitude)*32.0f);
        } else {
            D_L00_001B2750[i]=D_L00_001B2760[i]; D_L00_001B2758[i]=0;
            angle=func_002A78D8(0.52359885f,1.0471977f); sideangle=func_002A7878(-1.0471977f,0.17453295f);
            func_00282D30(&frame.toward,&frame.direction,scale); func_002ACEC0(&frame.toward,&frame.toward,&D_0018A560,sideangle); func_002ACEC0(&frame.toward,&frame.toward,&D_0018A570,angle);
            branch[0].q=frame.scratch.q; func_00282CC0(&branch[1],&frame.scratch,&frame.toward); scale=1;
            for(j=1;j<4;++j) { func_00282CF0(&frame.hit1,&branch[j],&D_L00_001B8DC0); angle=func_002A7878(0.17453295f,0.9599312f)*scale; scale=-scale; func_002ACEC0(&frame.toward,&frame.toward,&frame.hit1,angle); func_00282CC0(&branch[j+1],&branch[j],&frame.toward); }
        }
    }
    angle=func_002A78D8(0.52359885f,1.0471977f); sideangle=func_002A7878(-0.52359885f,1.0471977f);
    func_00282D30(&frame.toward,&frame.direction,D_L00_001AF1C8); func_002ACEC0(&frame.toward,&frame.toward,&D_0018A560,sideangle); func_002ACEC0(&frame.toward,&frame.toward,&D_0018A570,angle);
    rotation=func_002A7820(1,8); if(func_002A77E0(2)) rotation=-rotation; size=func_002A7878(0.2f,0.6f);
    func_002C2210(ctx,15,D_L00_001AF198|0x7F000000,0,rotation,&frame.toward,1,size*0.3f,size,0);
    func_002C2210(ctx,15,0x307F7F7F,0,-rotation,&frame.toward,1,size*0.15f,size*0.5f,0);
    rotation=func_002A7820(1,8); if(func_002A77E0(2)) rotation=-rotation; size=func_002A7878(0.15f,3.0f);
    func_002C2210(ctx+0x10,12,D_L00_001AF198|0x7F000000,0,rotation,&D_L00_001A8F20,1,size*0.1f,size,0);
}
