/* Medium-tier attempt: 632 bytes versus644; TI zero, register scheduling and two pre-div nops differ. */
typedef unsigned int VecBits __attribute__((mode(TI)));
typedef union { VecBits bits; float f[4]; } LocalVec;
extern void func_00282CC0(void *,void *,void *);
extern float func_00283180(float);
extern float func_00283198(float);
extern float func_00283CE0(int);
extern void func_002AB668(void *,void *,float);
extern void func_002ACDE8(void *,void *,void *);
extern void func_L00_002EBE88(void *,int,int,int,int,int,float,float,float);

void func_L00_003CD658(char *obj) {
    struct { LocalVec initial, axis, offset; VecBits ring[32]; } frame;
    char *ctx=*(char **)(obj+0x68);
    char *point=ctx;
    float angle=0.0f, step;
    int i=8;
    frame.initial.bits=0;
    frame.initial.f[3]=1.0f;
    frame.initial.f[2]=3.0f;
    frame.axis.f[0]=func_00283180(*(float *)(obj+0xF8));
    frame.axis.f[1]=func_00283198(*(float *)(obj+0xF8));
    frame.axis.f[2]=0.0f;
    frame.axis.f[3]=1.0f;
    step=6.283186f/(func_00283CE0(9)-1.0f);
    do {
        func_002AB668(&frame.offset,&frame.axis,angle);
        angle+=step;
        func_002ACDE8(point,&frame.initial,&frame.offset);
        func_00282CC0(point,point,obj+0x10);
        *(float *)(point+12)=0.6f;
        point+=16;
    } while(--i>=0);
    *(float *)(ctx+0x200)+=-0.1f;
    if(*(float *)(ctx+0x200)<=-7.0f) *(float *)(ctx+0x200)+=7.0f;
    func_L00_002EBE88(ctx,0x2B,0x2B,9,0x10400000,0x10400000,0.3f,0.6f,*(float *)(ctx+0x200));
    frame.offset.f[0]=func_00283180(*(float *)(obj+0xF8)+3.141593f)*0.65f;
    frame.offset.f[1]=func_00283198(*(float *)(obj+0xF8)+3.141593f)*0.65f;
    frame.offset.f[2]=0;
    point=ctx;
    for(i=0;i<9;i++) {
        frame.ring[i]=*(VecBits *)point;
        func_00282CC0(&frame.ring[i],&frame.offset,&frame.ring[i]);
        ((float *)&frame.ring[i])[3]=1.0f;
        point+=16;
    }
    func_L00_002EBE88(frame.ring,0x2B,0x2B,9,0x2200000,0x2200000,0.3f,0.6f,*(float *)(ctx+0x200));
}
