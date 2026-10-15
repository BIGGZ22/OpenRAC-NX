/* 564 bytes, retail 564. Two variants. Plain mode(TI) copy folds +0x10 source/destination offsets and chooses a0 for its quadword; retail uses zero-offset bases and v0. Callee-saved/FPU allocation and scheduling remain. Retail reuses the first particle for stores after the second allocation; preserved intentionally. */
typedef int Quad10 __attribute__((mode(TI)));
extern int func_00283CF0(float);
extern int func_002A77E0(int);
extern unsigned char *func_002BDCE8(void *,int,void *,int,int,float,float,float,float);
void func_L00_003EE288(unsigned char *p) {
    float vel[4], pos[4]; int n; unsigned char *first; unsigned char *second; unsigned char *third; unsigned char *particle;
    vel[0]=0.0f; vel[1]=-0.5f; vel[2]=0.0f; vel[3]=1.0f;
    n=func_002A77E0(16);
    if(func_002A77E0(2)) n=-n;
    *(Quad10 *)pos=*(Quad10 *)(p+0x10);
    first=func_002BDCE8(pos,n,vel,0xDF204080,0,0.1f,1.0f,0.9f,630000.0f);
    if(first) {
        *(unsigned short *)(first+0xA)=25;
        first[9]=func_00283CF0(4.0f)+112;
        particle=first+0x20;
        *(int *)(particle+4)=2;
        particle[0xA]=0xDF; particle[0xB]=first[0xA];
    }
    second=func_002BDCE8(pos,n,vel,0xDF204080,0,0.1f,1.0f,0.9f,630000.0f);
    if(second) {
        *(unsigned short *)(first+0xA)=25;
        first[9]=func_00283CF0(4.0f)+112;
        particle=second+0x20;
        *(int *)(particle+4)=2;
        particle[0xA]=0xDF; particle[0xB]=first[0xA];
    }
    third=func_002BDCE8(pos,16,vel,0xDFFFFFFF,0,0.05f,1.0f,0.97f,315000.0f);
    if(third) {
        int v=func_00283CF0(4.0f);
        *(unsigned short *)(third+0xA)=35;
        third[9]=v+112;
        third[8]=func_002A77E0(255);
        particle=third+0x20;
        particle[0xA]=0xDF; *(int *)(particle+4)=2;
        particle[0xB]=first[0xA];
    }
}
