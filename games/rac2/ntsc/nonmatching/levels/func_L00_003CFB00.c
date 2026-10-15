/* Unmatched plain-C reconstruction. 1260 bytes versus 1288; -G0.
 * instruction at +0x0 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern float D_L00_001B19CC,D_L00_001B19D0;
extern int func_002A9640(void *,int,int),func_002B3868(void *),func_00282948(void *);
extern void func_002A7750(void *,int,int),func_002833F0(void *,void *),func_0029F338(void *);
extern void func_002A7A00(void *,float,float),func_002A99A0(void *,int,void *,int,float),func_00282CC0(void *,void *,void *);
extern float func_002A7878(float,float);
extern int func_002A77E0(int),func_002A7820(int,int);
extern char *func_00273730(void *,int,int,int,void *,void *,void *,int,float,float);
extern void func_L00_0039B288(void *),func_002A0488(void *,int,int,int),func_002E5DA0(int,int,void *,int),func_L00_00326620(void *,int,int);
void func_L00_003CFB00(char *obj) {
    struct { Vec velocity,offset,position; } frame;
    char *ctx=*(char **)(obj+0x68),*p=ctx,*child,*childctx;
    int i,type,state;
    short classid;
    float life;
    switch((unsigned char)obj[0x20]) {
    case 0:
        p=ctx+0x3C; i=15;
        do { *(int *)p=0; --i; p-=4; } while(i>=0);
        if(*(short *)(obj+0xAA)==0xAC9) { *(float *)(obj+0xF4)+=0.78539824f; func_002833F0(obj+0xC0,obj+0xF0); }
        state=1; break;
    case 1:
        if(func_002A9640(obj,0x10000,0) && *(int *)(ctx+0x80)==0) func_002A7750(obj,2,-1);
        obj[0xA8]=0xFF; return;
    case 2:
        i=5; if(D_L00_001B19CC<0.85f && D_L00_001B19D0<0.85f) i=14;
        while(i) {
            func_002A7A00(&frame.offset,0.85f,1.95f);
            func_002A99A0(obj,1,&frame.position,0xFFFF,1000.0f);
            func_00282CC0(&frame.offset,&frame.offset,&frame.position);
            frame.velocity.f[2]=func_002A7878(0.0f,5.0f)*0.016666668f;
            frame.velocity.f[1]=func_002A7878(-6.0f,6.0f)*0.016666668f;
            type=2;
            if(func_002A77E0(4)) {
                type=1;
                *(float *)(p+0x40)=func_002A7878(20.0f,40.0f)*0.00027777778f;
                frame.velocity.f[0]=func_002A7878(-8.0f,-4.0f)*0.016666668f;
                life=func_002A7878(5.0f,10.0f);
            } else {
                *(int *)(p+0x40)=0;
                frame.velocity.f[0]=func_002A7878(2.0f,4.0f)*0.016666668f;
                life=func_002A7878(15.0f,20.0f);
            }
            child=func_00273730(obj,type,0x43E,func_002A77E0(4),&frame.offset,obj+0xF0,&frame.velocity,0xFF,180.0f,life);
            *(char **)p=child;
            if(child) {
                *(void (**)(void *))(child+0x64)=func_L00_0039B288;
                life=func_002A7878(0.75f,5.0f);
                *(float *)(*(char **)p+0x2C)=*(float *)(*(char **)(*(char **)p+0x24)+0x24)*life;
                childctx=*(char **)(*(char **)p+0x68);
                func_002A0488(*(char **)p,0x20,0x60,0x80);
                (*(char **)p)[0x23]=0x80;
                *(int *)(childctx+0x50)=0x1806020;
            }
            --i; p+=4;
        }
        func_002E5DA0(func_002A7820(0,1),0,obj,0xC0A);
        classid=*(short *)(obj+0xAA);
        switch(classid) {
        case 0xC0C: func_L00_00326620(obj,0xC12,0); break;
        case 0xC0A: func_L00_00326620(obj,0xC10,0); break;
        case 0xC0B: func_L00_00326620(obj,0xC11,0); break;
        case 0xC0D: func_L00_00326620(obj,0xC13,0); break;
        case 0xAC9: case 0xE01: case 0xE02: *(unsigned short *)(obj+0x7C)=0x8001; break;
        }
        state=3; break;
    case 3:
        if(func_002B3868(obj)) {
            if(!(*(unsigned short *)(obj+0x7C)&0x8000)) { obj[0x31]=0; *(unsigned short *)(obj+0x34)|=1; }
            *(int *)(obj+0x98)=0; *(int *)(ctx+0x84)=300;
        }
        if(func_00282948(ctx+0x84)) { if(!(*(unsigned short *)(obj+0x7C)&0x8000)) func_0029F338(obj); return; }
        i=15; do { --i; if(*(char **)p) *(float *)(*(char **)(*(char **)p+0x68)+4)-=*(float *)(p+0x40); p+=4; } while(i>=0);
        return;
    default: return;
    }
    func_002A7750(obj,state,-1);
}
