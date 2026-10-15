/* Unmatched plain-C reconstruction. 2444 bytes versus 2456; -G8.
 * instruction at +0x0 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern Vec D_00189EA0;
extern char D_001886D4[],D_L00_001B8DC0[],D_L00_001B8DD0[],*D_L00_001B8E10;
extern int D_001A7908;
extern float func_00283B98(float,float),func_00283198(float),func_00282C48(float),func_002A7940(void),func_00282E80(void *,void *),func_00282E48(void *,void *);
extern int func_00283CF0(float),func_L00_00324120(void *),func_002E59B0(void *,int),func_002E5CF8(int,int,void *),func_002ACBC8(void *,int);
extern void func_002A7750(void *,int,int),func_002E5DA0(int,int,void *,int),func_002E59F8(int),func_0029F338(void *),func_002A0F48(void *);
extern char *func_0029EFD0(int),*func_L00_002D70A0(int);
extern void func_L00_002D7140(void *),func_00335438(void *,void *,int,int,int),func_003353C8(void *),func_00335370(void *),func_003355A8(int);
extern void func_00282CF0(void *,void *,void *),func_002AD6F0(void *,void *,void *,void *);
extern void func_002AA3A8(void *,void *,float,float,float,float),func_002AA548(void *,void *,float,float,float,float);
#define I(p,o) (*(int *)((p)+(o)))
#define F(p,o) (*(float *)((p)+(o)))
#define P(p,o) (*(char **)((p)+(o)))
#define STOP_SOUND() { handle=I(ctx,0x88); if(handle!=-1 && P(D_001886D4+handle*0x70,0x14)==obj && D_001886D4[handle*0x70]) func_002E59F8(handle); I(ctx,0x88)=-1; }
#define RELEASE() if(I(ctx,0xA4)) { func_L00_002D7140(P(ctx,0x9C)); I(ctx,0xA4)=0; }
#define START_SOUND(s) if(!func_002E59B0(obj,I(ctx,0x88))) I(ctx,0x88)=func_002E5CF8(s,4,obj)
#define RESET_CHILD() func_002A7750(obj,1,-1); I(P(ctx,0xA0),0x78)=0x808C8C8C; I(P(ctx,0xA0),0x98)=I(P(P(ctx,0xA0),0x24),0x10)
void func_L00_003CD8E0(char *obj) {
    struct { Vec old,rotation,delta,camera; } frame;
    char *ctx=P(obj,0x68),*child;
    int value,handle;
    unsigned int brightness;
    frame.old.q=*(Quad *)(obj+0x10); frame.rotation.q=*(Quad *)(obj+0xF0);
    switch((unsigned char)obj[0x20]) {
    case 0:
        *(unsigned short *)(obj+0x34)|=0x800;
        I(ctx,0x5C)|=1; *(Quad *)(ctx+0x60)=*(Quad *)(obj+0x10); I(ctx,0xAC)=0; obj[0x20]=1;
        if(!I(ctx,0x90)) { F(ctx,0x94)=F(ctx,0x68); F(ctx,0x98)=F(ctx,0x8C); I(ctx,0x80)=0; }
        else { I(ctx,0x80)=1; F(ctx,0x94)=F(ctx,0x8C); F(ctx,0x98)=F(ctx,0x68); }
        I(ctx,0x84)=0; I(ctx,0x88)=-1; child=func_0029EFD0(0xC16); P(ctx,0xA0)=child;
        if(!child) { func_0029F338(obj); return; }
        child[0x30]=obj[0x30]; *(short *)(P(ctx,0xA0)+0x32)=*(short *)(obj+0x32); P(ctx,0xA0)[0x31]=1;
        child=P(ctx,0xA0); *(Quad *)(child+0x10)=*(Quad *)(obj+0x10); F(child,0x18)+=0.25f;
        func_002A0F48(P(ctx,0xA0)); F(ctx,0xA8)=func_002A7940(); break;
    case 1:
        F(ctx,0xA8)=func_00283B98(F(ctx,0xA8),0.10471977f);
        value=func_00283CF0((func_00283198(F(ctx,0xA8))*4.0f-3.0f)*208.0f);
        if(value<209) { if(value<80) value=80; } else value=208;
        brightness=value; I(P(ctx,0xA0),0x78)=brightness<<16|brightness<<8|0x80000000|brightness;
        if(func_L00_00324120(P(ctx,0xA0)) && I(ctx,0x80)) {
            func_002A7750(obj,2,-1); I(ctx,0xAC)=0; I(P(ctx,0xA0),0x78)=0x8000DF2A;
            if(!D_001A7908) {
                func_00335438(D_L00_001B8DC0,D_L00_001B8DD0,2,120,0);
                frame.camera.f[2]=D_00189EA0.f[2]+1.5f; frame.camera.f[0]=246.85f; frame.camera.f[1]=198.19f;
                frame.delta.f[2]=1.5707965f; frame.delta.f[0]=0; frame.delta.f[1]=0;
                func_003353C8(&frame.delta); func_00335370(&frame.camera);
            }
            func_002E5DA0(0,0,obj,0x8A1); I(P(ctx,0xA0),0x98)=0; START_SOUND(0);
            P(ctx,0x9C)=func_L00_002D70A0(5); F(P(ctx,0x9C),0)=2.0f; F(P(ctx,0x9C),4)=1.5f; I(ctx,0xA4)=1;
        }
        if(func_00282C48(F(obj,0x18)-D_00189EA0.f[2])>12.0f && !I(ctx,0x80)) { START_SOUND(0); func_002A7750(obj,3,-1); }
        else if(func_00282C48(F(obj,0x18)-D_00189EA0.f[2])>12.0f && I(ctx,0x80)==1) { START_SOUND(1); func_002A7750(obj,2,-1); I(ctx,0xAC)=0; }
        else break;
        I(P(ctx,0xA0),0x78)=0x8000DF2A; I(P(ctx,0xA0),0x98)=0; break;
    case 2:
        frame.delta.q=*(Quad *)(obj+0x10); frame.delta.f[2]=F(ctx,0x94);
        if(func_00282E48(obj+0x10,&frame.delta)>0.14f) { func_002AA548(obj+0x18,ctx+0xAC,frame.delta.f[2],0.0019444445f,0.0016666667f,0.10000001f); ++I(ctx,0x84); }
        else if(func_00282E80(obj+0x10,&D_00189EA0)>1.2f) { RELEASE(); I(ctx,0x80)=0; STOP_SOUND(); RESET_CHILD(); }
        else { if(I(ctx,0x84)>0) { func_002E5CF8(1,0,obj); I(ctx,0x84)=0; } RELEASE(); STOP_SOUND(); }
        if(func_00282C48(F(obj,0x18)-D_00189EA0.f[2])>3.0f && D_00189EA0.f[2]<F(obj,0x18)) func_002A7750(obj,3,-1);
        break;
    case 3:
        if(F(ctx,0x98)<F(obj,0x18)) {
            if(!func_002ACBC8(&D_00189EA0,I(ctx,0xB0))) F(obj,0x18)-=0.08f;
            else { if(F(obj,0x18)-D_00189EA0.f[2]<1.8f && D_00189EA0.f[2]<F(obj,0x18)) break; func_002AA3A8(obj+0x18,ctx+0xAC,F(ctx,0x98),0.02f,0.3f,0.10000001f); }
            ++I(ctx,0x84);
        } else if(func_00282E80(obj+0x10,&D_00189EA0)<=1.0f) { RELEASE(); STOP_SOUND(); }
        else { func_002E5CF8(1,0,obj); RELEASE(); I(ctx,0x80)=1; STOP_SOUND(); RESET_CHILD(); I(ctx,0x84)=0; }
        break;
    }
    func_00282CF0(&frame.delta,obj+0x10,&frame.old); func_002AD6F0(ctx+0x20,&frame.delta,&frame.rotation,obj+0xF0);
    child=P(ctx,0xA0); *(Quad *)(child+0x10)=*(Quad *)(obj+0x10); F(child,0x18)+=0.25f;
    if(*(short *)(D_L00_001B8E10+0x86)==5) {
        frame.camera.f[2]=D_00189EA0.f[2]+1.5f; frame.camera.f[1]=198.19f; frame.camera.f[0]=246.85f;
        func_00335370(&frame.camera); if(func_00282E80(D_L00_001B8E10+0x30,&frame.camera)<0.5f) func_003355A8(1);
    }
}
