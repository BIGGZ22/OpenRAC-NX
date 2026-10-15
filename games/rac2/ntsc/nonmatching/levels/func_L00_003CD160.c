/* Unmatched plain-C reconstruction. 1188 bytes versus 1268; -G0.
 * instruction at +0x4c differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern Vec D_00189EA0;
extern char *D_0018C0B0;
extern char D_001886D4[];
extern Vec D_L00_001BF220;
extern float func_00283180(float),func_00283198(float),func_00283CE0(int);
extern void func_002AB668(void *,void *,float);
extern void func_002ACDE8(void *,void *,void *);
extern void func_00282CC0(void *,void *,void *);
extern int func_00282948(void *),func_002ACBC8(void *,int);
extern void func_L00_003CD658(void *),func_L00_002E6CF0(void (*)(void *),void *);
extern int func_00277258(void *,int,void *,float);
extern int func_002E59B0(void *,int),func_L00_0035F5A0(int,int,void *);
extern float func_002A7878(float,float),func_002A78D8(float,float),func_002A7940(void);
extern int func_002A77E0(int);
extern void func_002AF000(void *,float,float,float);
extern void func_002C2210(void *,int,int,int,int,void *,int,float,float,float);
extern void func_002E59F8(int);
void func_L00_003CD160(char *obj) {
    struct { Vec offset,direction,rotated,position; } frame;
    char *ctx=*(char **)(obj+0x68),*p;
    int i,handle,sign;
    float angle,step,next,size,yaw,pitch,speed;
    if((unsigned char)obj[0x20]==0) {
        *(float *)(obj+0x18)+=1.5f;
        *(int *)(ctx+0x200)=0;
        *(Quad *)(ctx+0x210)=*(Quad *)(obj+0x10);
        *(int *)(ctx+0x204)=0;
        frame.offset.q=0; frame.offset.f[3]=1.0f; frame.offset.f[2]=3.0f;
        frame.direction.f[0]=func_00283180(*(float *)(obj+0xF8));
        frame.direction.f[1]=func_00283198(*(float *)(obj+0xF8));
        frame.direction.f[3]=1.0f; frame.direction.f[2]=0.0f;
        step=6.283186f/(func_00283CE0(9)-1.0f);
        p=ctx; i=8; angle=0.0f;
        do {
            next=angle+step;
            func_002AB668(&frame.rotated,&frame.direction,angle);
            --i;
            func_002ACDE8(p,&frame.offset,&frame.rotated);
            func_00282CC0(p,p,obj+0x10);
            *(float *)(p+0xC)=0.6f;
            p+=0x10; angle=next;
        } while(i>=0);
        obj[0x20]=1;
    }
    if((unsigned char)obj[0x20]==1 && func_00282948(ctx+0x208)) obj[0x20]=2;
    if((unsigned char)obj[0x20]==2) {
        frame.offset.f[0]=func_00283180(*(float *)(obj+0xF8))*0.083333336f;
        frame.offset.f[1]=func_00283198(*(float *)(obj+0xF8))*0.083333336f;
        frame.offset.f[2]=0.0f;
        func_00282CC0(obj+0x10,&frame.offset,obj+0x10);
        if(++*(int *)(ctx+0x204)>0x207) {
            *(int *)(ctx+0x204)=0;
            *(Quad *)(obj+0x10)=*(Quad *)(ctx+0x210);
        }
        if(func_002ACBC8(&D_00189EA0,*(int *)(ctx+0x220))) func_L00_002E6CF0(func_L00_003CD658,obj);
    }
    if(func_002ACBC8(&D_00189EA0,*(int *)(ctx+0x20C))) {
        frame.offset.q=D_00189EA0.q; frame.offset.f[2]+=0.75f;
        if(func_00277258(&frame.offset,2,D_0018C0B0,0.5f)) {
            if(!func_002E59B0(D_0018C0B0,*(int *)(ctx+0x224))) *(int *)(ctx+0x224)=func_L00_0035F5A0(0x18,4,D_0018C0B0);
            frame.position.q=D_L00_001BF220.q;
            i=0;
            do {
                size=func_002A7878(1.0f,2.0f);
                sign=func_002A77E0(2); if(!sign) sign=-1;
                yaw=func_002A7940(); pitch=func_002A7878(0.3490659f,1.53589f); speed=func_002A7878(0.05f,0.1f);
                func_002AF000(&frame.direction,speed,yaw,pitch);
                frame.rotated.q=frame.position.q;
                frame.position.f[0]+=func_002A78D8(0.0f,0.15f); --i;
                frame.position.f[1]+=func_002A78D8(0.0f,0.15f);
                frame.position.f[2]+=func_002A78D8(0.0f,0.15f);
                func_002C2210(&frame.position,15,0xFF400000,0,sign,&frame.direction,0,size*0.1f,size,0.006f);
                func_002C2210(&frame.position,15,0x3F7F7F7F,1,-sign,&frame.direction,0,size*0.04f,size*0.4f,0.006f);
            } while(i>=0);
            return;
        }
    }
    handle=*(int *)(ctx+0x224);
    if(handle!=-1 && *(char **)(D_001886D4+handle*0x70+0x14)==D_0018C0B0 && D_001886D4[handle*0x70]) func_002E59F8(handle);
    *(int *)(ctx+0x224)=-1;
}
