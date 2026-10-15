/* Medium-tier attempt: 920 bytes versus924; bank absolute/GP access and TI copy reloads differ. */
typedef unsigned int UpdateBits __attribute__((mode(TI)));
#define D_00189EA0 (D_00189E20+0x80)
#define D_00189EB0 (D_00189E20+0x90)
extern unsigned char D_001395B8;
extern char D_00189E20[];
extern unsigned char D_0019B2E8[];
extern int D_001A79F0 __attribute__((sda));
extern char D_L00_001A8F00[];
extern char *D_L00_001B1F8C;
extern int D_L00_001FA920[];
extern void func_00282CC0(void *, void *, void *);
extern void func_00282F20(void *, void *, float);
extern void func_002833F0(void *,void *);
extern void func_00283410(void *,void *);
extern float func_00283B98(float,float);
extern float func_00283C80(float,float);
extern void func_002A0430(int);
extern void func_002A7750(void *,int,int);
extern int func_002A8508(int,int);
extern void func_002AA800(void *,int,float,float);
extern int func_002B3868(void *);
extern void func_002DF3F8(void *,void *);
extern void func_002E5CF8(int,int,void *);
extern void func_L00_002B9D60(void *,void *,int,int);
extern void func_L00_002D40A0(void *,int,float);
extern void func_L00_003294F0(void *,int);
extern void func_L00_00429020(int);
extern int func_L00_00429048(void);

void func_L00_003CF610(unsigned char *obj) {
    UpdateBits second,first;
    int *ctx=*(int **)(obj+0x68);
    if(!ctx) return;
    if((unsigned char)obj[0x20]==0) {
        if(D_L00_001FA920[ctx[0]]==0) {
            if(ctx[2]) { obj[0x20]=3; return; }
            if(!ctx[5]) { obj[0x20]=2; return; }
            *(float *)(ctx+6)=*(float *)(obj+0xF4);
            *(float *)(obj+0xF4)=func_00283B98(*(float *)(obj+0xF4),-1.5707965f);
            func_002A7750(obj,4,-1);
            return;
        }
        if(func_002A8508(ctx[0],-1)==0) {
            if((D_0019B2E8[(unsigned char)obj[0xB0]+D_001A79F0*16]&0x80)==0) {
                obj[0x20]=1;
                if(!ctx[2]) return;
                D_001395B8=1;
                func_L00_00429020(ctx[4]);
                func_002A0430((unsigned char)obj[0xB0]);
                first=*(UpdateBits *)(D_L00_001B1F8C+ctx[3]*128+0x30);
                second=*(UpdateBits *)(D_L00_001B1F8C+ctx[3]*128+0x70);
                func_002DF3F8(&first,&second);
                return;
            }
            obj[0x20]=3;
        } else if(D_0019B2E8[(unsigned char)obj[0xB0]+D_001A79F0*16]&0x80) obj[0x20]=3;
    }
    if((unsigned char)obj[0x20]==1) {
        if(ctx[4]) {
            if(func_L00_00429048()) {
                *(UpdateBits *)D_00189EA0=*(UpdateBits *)(D_L00_001B1F8C+ctx[3]*128+0x30);
                *(UpdateBits *)D_00189EB0=*(UpdateBits *)(D_L00_001B1F8C+ctx[3]*128+0x70);
                obj[0x20]=3;
                func_L00_003294F0(obj,1);
            }
        }
    }
    if((unsigned char)obj[0x20]==3) {
        if(*(float *)(ctx+1)==0.0f) func_002E5CF8(0,0,obj);
        *(float *)(obj+0xF4)+=-0.017453294f;
        func_002833F0(obj+0xC0,obj+0xF0);
        *(float *)(ctx+1)+=1.0f;
        if(*(float *)(ctx+1)>135.0f) { func_002E5CF8(1,0,obj); obj[0x20]=2; }
    }
    if((unsigned char)obj[0x20]!=4) return;
    if(*(int *)D_L00_001A8F00) return;
    if(func_002B3868(obj)) {
        func_002E5CF8(1,0,obj);
        second=*(UpdateBits *)D_00189EA0;
        func_00283410(D_00189E20,D_00189EB0);
        func_00282F20(&first,D_00189E20,-2.5f);
        func_00282CC0(&first,&second,&first);
        func_L00_002B9D60(&first,D_00189EB0,0,1);
        func_L00_002D40A0(&second,1,*(float *)(D_00189EB0+8));
    }
    func_002AA800(obj+0xF4,0,*(float *)(ctx+6),0.02618f);
    if(func_00283C80(*(float *)(obj+0xF4),*(float *)(ctx+6))<0.001f) obj[0x20]=2;
}
