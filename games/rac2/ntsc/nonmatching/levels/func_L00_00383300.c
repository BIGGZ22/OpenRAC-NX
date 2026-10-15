/* Unmatched plain-C reconstruction. 1576 bytes versus 1620; -G0.
 * instruction at +0x8 differs. Full behavior and linked-image equality remain unverified.
 */
typedef unsigned int Quad __attribute__((mode(TI)));
typedef union { Quad q; float f[4]; } Vec;
extern char *D_L00_001B1F8C,*D_L00_001FDA80[],*D_L00_00220DA0[];
extern int D_L00_001B1A50,D_L00_001B19C8,D_L00_001A8F00,D_0018C0B4,D_0018C2EC,D_L00_001BF154;
extern unsigned int D_00139788,D_001A7424;
extern float D_L00_001B18E0,D_L00_001B24B0 __attribute__((sda)),D_L00_001B24B4,D_L00_001B24B8;
extern char D_00189EA0[],D_L00_001BEFC0[],D_L00_001AED10[];
extern int func_002F5D10(void),func_00288D58(void),func_002B3868(void *),func_L00_00429048(int),func_L00_002D40F0(void),func_0029B3E0(int,int),func_002ACBC8(void *,int);
extern void func_L00_002B9D60(void *,void *,int,int),func_002E6178(void *),func_L00_00429020(int),func_002A7750(void *,int,int),func_002ED260(int),func_002AD7B8(int),func_L00_003672F8(void),func_002B37E8(void *,void *);
extern char *func_0029EFD0(int);
extern float func_00283248(float,float),func_00283B98(float,float),func_00283180(float),func_00283198(float),func_00282E80(void *,void *);
extern void func_002833F0(void *,void *),func_00288AA0(void *,int),func_00288BE8(void *),func_002F5CC8(void *),func_L00_0035F5A0(int,int,void *),func_L00_002E6CF0(void (*)(void *),void *),func_L00_003618D8(void *),func_0026FEB8(void *),func_00282CC0(void *,void *,void *),func_L00_002D40A0(void *,int,float),func_L00_0042A500(int),func_00282CF0(void *,void *,void *),func_00282F20(void *,void *,float),func_L00_00383958(void *);
#define I(p,o) (*(int *)((p)+(o)))
#define F(p,o) (*(float *)((p)+(o)))
#define P(p,o) (*(char **)((p)+(o)))
void func_L00_00383300(char *obj) {
    Vec offset;
    char *ctx=P(obj,0x68),*entry,*path,*child,**list;
    int state;
    float angle;
    switch((unsigned char)obj[0x20]) {
    case 0:
        if(func_00288D58()) func_002AD7B8(0x18);
        func_L00_003672F8();
        if(I(ctx,4)!=-1) { entry=D_L00_001B1F8C+I(ctx,4)*0x80; func_L00_002B9D60(entry+0x30,entry+0x70,0,1); }
        child=func_0029EFD0(0xD54); P(ctx,0x18)=child; func_002B37E8(child,obj);
        child=P(ctx,0x18); path=D_L00_001FDA80[I(ctx,0x14)]; *(Quad *)(child+0x10)=*(Quad *)(path+0x10);
        func_002E6178(child);
        child=P(ctx,0x18); *(Quad *)(child+0x10)=*(Quad *)(path+0x10); *(short *)(child+0x32)=0x28F;
        F(P(ctx,0x18),0xF8)=func_00283248(F(path,0x20)-F(obj,0x10),F(path,0x24)-F(obj,0x14));
        func_002833F0(P(ctx,0x18)+0xC0,P(ctx,0x18)+0xF0); I(ctx,0x20)=0; obj[0x30]=0xFF;
        if(!(D_00139788&2) && D_0018C0B4!=0x1C && D_0018C2EC>0 && !D_L00_001A8F00 && (I(ctx,0)==-1 || func_002ACBC8(D_00189EA0,I(ctx,0)))) {
            func_00288AA0(D_L00_001BEFC0,0x21); func_00288BE8(D_L00_001BEFC0);
            if(D_L00_001B19C8<5) func_002F5CC8(obj);
            state=1;
        } else state=4;
        func_002A7750(obj,state,-1); break;
    case 1:
        if(!func_002F5D10()) {
            if(I(ctx,4)!=-1) { entry=D_L00_001B1F8C+I(ctx,4)*0x80; func_L00_002B9D60(entry+0x30,entry+0x70,0,1); }
            func_002E6178(P(ctx,0x18)); func_L00_00429020(I(ctx,8)); D_L00_001B1A50=0; D_L00_001B18E0=1.5f;
            func_002A7750(obj,2,-1); func_002ED260(D_L00_001BF154+0xF0000);
        } break;
    case 2:
        if(func_002B3868(obj)) { func_L00_0035F5A0(0x17,0,0); D_L00_001B1A50=0; }
        angle=func_00283B98(F(P(ctx,0x18),0xF8),3.1415927f);
        D_L00_001B24B0=func_00283180(angle)*0.6f; D_L00_001B24B4=func_00283198(angle)*0.6f; D_L00_001B24B8=0.0f;
        P(ctx,0x18)[0x95]=0xFF; func_L00_002E6CF0(func_L00_003618D8,P(ctx,0x18));
        ++*(unsigned short *)(obj+0x96);
        if(*(unsigned short *)(obj+0x96)>10 && func_L00_00429048(I(ctx,8))) {
            func_0026FEB8(D_L00_001AED10);
            *(unsigned short *)(P(ctx,0x18)+0x34)|=0x41; *(unsigned short *)(P(ctx,0x18)+0x34)&=0xEFFF; I(P(ctx,0x18),0x98)=0;
            offset.f[0]=func_00283180(F(D_L00_001B1F8C+I(ctx,4)*0x80,0x78)); offset.f[0]+=offset.f[0];
            offset.f[1]=func_00283198(F(D_L00_001B1F8C+I(ctx,4)*0x80,0x78)); offset.f[1]+=offset.f[1]; offset.f[2]=0;
            func_00282CC0(&offset,&offset,D_L00_001B1F8C+I(ctx,4)*0x80+0x30);
            func_L00_002D40A0(&offset,1,F(D_L00_001B1F8C+I(ctx,4)*0x80,0x78));
            func_L00_0042A500(I(ctx,0x10)); D_L00_001B1A50=2; func_002A7750(obj,4,-1);
        }
        path=D_L00_001FDA80[I(ctx,0x14)];
        if(func_00282E80(P(ctx,0x18)+0x10,path+I(ctx,0x20)*16+16)<0.75f) ++I(ctx,0x20);
        if(I(path,0)-1<I(ctx,0x20)) I(ctx,0x20)=I(path,0)-1;
        func_00282CF0(&offset,path+I(ctx,0x20)*16+16,P(ctx,0x18)+0x10);
        func_00282F20(&offset,&offset,F(ctx,0x1C)*0.016666668f);
        func_00282CC0(P(ctx,0x18)+0x10,P(ctx,0x18)+0x10,&offset); break;
    case 4:
        if(func_002B3868(obj)) { D_L00_001B1A50=2; func_L00_0042A500(I(ctx,0x10)); }
        if(!func_L00_002D40F0()) { if(func_0029B3E0(1,-1)) D_001A7424|=0x200; func_002A7750(obj,5,-1); } break;
    }
    if((unsigned int)(unsigned char)obj[0x20]-4<2) {
        list=D_L00_00220DA0;
        while(*list) { *(unsigned short *)(*list+0x34)|=0x41; *(unsigned short *)(*list+0x34)&=0xEFFF; child=*list++; I(child,0x98)=0; }
    } else {
        path=D_L00_001FDA80[I(ctx,0x14)]; func_L00_00383958(P(ctx,0x18));
        F(P(ctx,0x18),0xF8)=func_00283248(F(path,0x20)-F(P(ctx,0x18),0x10),F(path,0x24)-F(P(ctx,0x18),0x14));
        func_002833F0(P(ctx,0x18)+0xC0,P(ctx,0x18)+0xF0);
    }
}
