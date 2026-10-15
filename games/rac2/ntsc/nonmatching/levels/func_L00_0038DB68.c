/* Medium-tier attempt: 740 bytes versus776; TI zero materialization and collision base lifetime differ. */
typedef unsigned int VecBits __attribute__((mode(TI)));
typedef union { VecBits bits; float f[4]; } LocalVec;
extern unsigned char D_00139574;
extern char *D_0018C0B0;
extern int D_L00_001AC850;
extern char D_L00_001BF200[];
extern char D_L00_00262570[];
extern int func_002761F8(void *,void *,int,void *,void *);
extern int func_00282948(char *);
extern void func_00282CC0(void *,void *,void *);
extern void func_00282D30(void *,void *,float);
extern void func_00282F20(void *,void *,float);
extern void func_00282F70(void *,void *,float);
extern void func_00283028(void *,void *,void *);
extern void func_002AFFD8(void *,void *,int);
extern void func_002FEE50(int);
extern void func_L00_0038DE70(void *,void *,void *);
extern void func_L00_0038E880(void *,void *,void *);
extern void func_L00_0038F3D8(void *);

void func_L00_0038DB68(char *obj) {
    struct { LocalVec velocity, normal, old, surface;
    struct { LocalVec vector; char *moby; int flags; unsigned char kind, active; unsigned short class_id; float material; int enabled; } packet; } frame;
    char *ctx=*(char **)(obj+0x68);
    char *position=obj+0x10;
    char *hit;
    frame.velocity.bits=0;
    frame.velocity.f[2]=0.033333335f;
    frame.normal.bits=0;
    frame.normal.f[2]=1.0f;
    frame.old.bits=*(VecBits *)position;
    func_00282CC0(position,position,ctx);
    func_002AFFD8(position,&frame.surface,0);
    func_00282D30(&frame.surface,&frame.surface,0.0003055556f);
    func_L00_0038F3D8(obj);
    frame.packet.moby=obj; frame.packet.flags=0x830000;
    if(*(int *)(ctx+0x18)<0x122) frame.packet.flags=0x830001;
    frame.packet.enabled=1;
    frame.packet.material=*(float *)(D_L00_00262570+D_00139574*0xE0+0x70);
    frame.packet.vector.bits=*(VecBits *)ctx;
    func_00282F70(&frame.packet.vector,&frame.packet.vector,1.0f);
    frame.packet.class_id=*(unsigned short *)(obj+0xAA);
    frame.packet.kind=2; frame.packet.vector.f[3]=5627.925f; frame.packet.vector.f[2]=1.0f; frame.packet.active=1;
    (*(int *)(ctx+0x14))++;
    if(func_002761F8(&frame.old,position,0x10,obj,&frame.packet)) {
        hit=*(char **)(D_L00_001BF200+0x18);
        if(hit) {
            if((hit==D_0018C0B0 || *(short *)(hit+0xAA)==0xC0) && *(int *)(ctx+0x18)>=0x122) goto finish;
            if(*(int *)(hit+0x64)==D_L00_001AC850 && *(short *)(hit+0xAA)!=0x336) goto finish;
            obj[0xBC]=1;
            *(VecBits *)(obj+0x10)=*(VecBits *)(D_L00_001BF200+0x20);
            func_00283028(&frame.velocity,ctx,D_L00_001BF200+0x40);
            func_00282F20(&frame.velocity,&frame.velocity,0.033333335f);
        } else {
            if(*(int *)(D_L00_001BF200+0x1C)<=0) goto finish;
            obj[0xBC]=1;
            *(VecBits *)position=*(VecBits *)(D_L00_001BF200+0x20);
            func_00283028(&frame.velocity,ctx,D_L00_001BF200+0x40);
            func_00282F20(&frame.velocity,&frame.velocity,0.033333335f);
        }
        func_00282F20(&frame.normal,D_L00_001BF200+0x40,1.0f);
    } else if(func_00282948(ctx+0x18)) {
        obj[0xBC]=1;
        func_00282CC0(&frame.velocity,&frame.velocity,ctx);
    }
finish:
    if((unsigned char)obj[0xBC]) {
        func_002FEE50(*(int *)(ctx+0x1C));
        if(*(int *)(ctx+0x14)<20) func_L00_0038DE70(obj,&frame.velocity,&frame.normal);
        else func_L00_0038E880(obj,&frame.velocity,&frame.normal);
    }
}
