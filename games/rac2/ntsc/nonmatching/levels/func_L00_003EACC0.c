/* 560 bytes, retail 568. Two variants. Plain mode(TI) copies fold source/destination +0x10 offsets; retail materialises zero-offset bases. State/object registers and FPU scheduling also differ. */
extern int func_00282948(int *);
extern int func_002A7820(int,int);
extern float func_00283180(float);
extern float func_00283198(float);
extern float func_002A78D8(float,float);
extern float func_002A7878(float,float);
extern int func_00283CF0(float);
extern void func_L00_00338E90(void *,void *,int,int,int,float);
typedef int Quad10 __attribute__((mode(TI)));
void func_L00_003EACC0(unsigned char *p) {
    float v[4]; float pos[4]; float r;
    int *s=*(int **)(p+0x68);
    unsigned char *base;
    if ((unsigned char)p[0x20]==0) {
        p[0x30]=0x80; p[0x20]=1; s[1]=20; s[0]=0;
    }
    if ((unsigned char)p[0x20]==1) {
        if(func_00282948(s)) {
            p[0x20]=2; s[0]=func_002A7820(50,120);
        }
    }
    if ((unsigned char)p[0x20]==2) {
        if(func_00282948(s+1)) {
            p[0x20]=1; s[1]=func_002A7820(10,25);
        } else {
            v[0]=func_00283180(*(float *)(p+0xf8)-1.570796489715576171875f)*0.416666686534881591797f;
            v[1]=func_00283198(*(float *)(p+0xf8)-1.570796489715576171875f)*0.416666686534881591797f;
            v[2]=0.0f;
            base=p+0x10;
            *(Quad10 *)pos=*(Quad10 *)base;
            pos[2]+=func_002A78D8(v[2],0.5f);
            r=func_002A7878(0.1f,3.0f);
            func_L00_00338E90(pos,v,300,func_00283CF0(func_002A78D8(1.0f,4.0f)),0x40808080,r);
            *(Quad10 *)pos=*(Quad10 *)base;
            pos[2]+=func_002A78D8(v[2],0.5f);
            r=func_002A7878(0.1f,3.0f);
            func_L00_00338E90(pos,v,300,func_00283CF0(func_002A78D8(1.0f,4.0f)),0x40808080,r);
        }
    }
}
