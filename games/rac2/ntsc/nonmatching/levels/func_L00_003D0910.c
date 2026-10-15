/* Unmatched plain-C reconstruction. 2348 bytes versus 2376; -G0.
 * instruction at +0x4 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern float D_L00_001B0168 __attribute__((sda)),D_L00_001B016C __attribute__((sda)),D_L00_001B0170 __attribute__((sda)),D_L00_001B0174 __attribute__((sda)),D_L00_001B0178 __attribute__((sda));
extern int func_002A77E0(int),func_00282948(void *),func_00283CF0(float);
extern void func_00282F20(void *,void *,float),func_00282CC0(void *,void *,void *);
extern char *func_002BDCE8(void *,int,void *,int,int,float,float,float,float);
extern void func_002AEE70(void *,void *,void *,float,float,float),func_002AEF28(void *,void *,void *,float,float,float);
#define PARTICLE_PAIR()      first=func_002BDCE8(&frame.position,angle,&frame.velocity,0xDF204080,0,0.1f,1.0f,0.9f,100000.0f);      if(first) { *(short *)(first+10)=20; first[9]=func_00283CF0(4.0f)+112; *(int *)(first+0x24)=2; first[0x2A]=0xDF; first[0x2B]=first[10]; }      child=func_002BDCE8(&frame.position,angle,&frame.velocity,0xDF204080,0,0.1f,1.0f,0.9f,100000.0f);      if(child) { *(short *)(first+10)=20; first[9]=func_00283CF0(4.0f)+112; *(int *)(child+0x24)=2; child[0x2A]=0xDF; child[0x2B]=first[10]; }      child=func_002BDCE8(&frame.position,16,&frame.velocity,0xDFFFFFFF,0,0.05f,1.0f,0.97f,60000.0f);      if(child) { int brightness=func_00283CF0(4.0f); *(short *)(child+10)=20; child[9]=brightness+112; child[8]=func_002A77E0(255); child[0x2A]=0xDF; *(int *)(child+0x24)=2; child[0x2B]=first[10]; }
#define EMIT_EXHAUST()      func_00282F20(&frame.velocity,obj+0xC0,-0.15f);      angle=func_002A77E0(16); if(func_002A77E0(2)) angle=-angle;      frame.position.q=*(Quad *)(obj+0x10);      func_00282F20(&frame.forward,obj+0xC0,-2.3f);      func_00282CC0(&frame.position,&frame.position,&frame.forward);      func_00282F20(&frame.up,obj+0xE0,0.75f);      func_00282CC0(&frame.position,&frame.position,&frame.up);      func_00282F20(&frame.side,obj+0xD0,0.25f);      func_00282CC0(&frame.position,&frame.side,&frame.position);      PARTICLE_PAIR();      angle=func_002A77E0(16); if(func_002A77E0(2)) angle=-angle;      func_00282F20(&frame.side,obj+0xD0,-0.5f);      func_00282CC0(&frame.position,&frame.side,&frame.position);      PARTICLE_PAIR();
void func_L00_003D0910(char *obj) {
    struct { Vec velocity,forward,position,up,side; } frame;
    char *ctx=*(char **)(obj+0x68),*first,*child;
    int angle;
    if((unsigned char)obj[0x20]==0) {
        *(int *)(ctx+0x10)=func_002A77E0(40);
        obj[0x30]=100; obj[0x20]=1; *(int *)(ctx+0x18)=0; *(float *)(ctx+0x28)=*(float *)(obj+0x18);
    }
    if((unsigned char)obj[0x20]==1) {
        if(func_00282948(ctx+0x10)) obj[0x20]=2;
        EMIT_EXHAUST();
    }
    if((unsigned char)obj[0x20]==2) {
        func_002AEE70(obj,ctx+0xC,ctx,D_L00_001B0168,D_L00_001B016C*0.00029088825f,*(float *)(ctx+0x28));
        func_002AEF28(obj,ctx+4,ctx+8,D_L00_001B0178*0.017453294f,D_L00_001B0170*0.00029088825f,D_L00_001B0174*0.00029088825f);
        EMIT_EXHAUST();
    }
}
