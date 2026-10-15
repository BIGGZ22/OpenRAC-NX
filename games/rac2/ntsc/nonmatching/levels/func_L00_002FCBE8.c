/* Medium-tier attempt: 452 bytes versus492; base lifetime and repeated loop references differ. */
typedef struct {
    char pad0[0x30]; int state, timer; char pad38[12]; short count; char pad46[10]; char *out; char pad54[0x138]; char *objects[1];
} Context;
extern int D_001A79F0 __attribute__((sda));
extern float D_L00_001B24B0[4];
extern Context D_L00_001BCAC0;
extern void func_00282C88(void *);
extern void func_002E64A0(void *);
extern void func_002ED260(void *);
extern void func_L00_002E6CF0(void (*)(void));
extern void func_L00_002FC980(void);
extern void func_L00_00326048(void *, void *, unsigned int, unsigned int, float);
extern void func_L00_003618D8(void);

void func_L00_002FCBE8(void) {
    int i=0;
    Context *ctx;
    char **objects;
    func_L00_002FC980();
    ctx=&D_L00_001BCAC0;
    if(ctx->count>0) {
        objects=ctx->objects;
        do {
            char *obj=*objects;
            if (obj && *(short *)(obj+0xAA)==0xE29) *(unsigned short *)(obj+0x34)|=0x800;
            if (*(short *)(obj+0xAA)==0xD54) func_002E64A0(obj);
            if (obj && *(short *)(obj+0xAA)==4000 && *(char **)(obj+0x68)) {
                func_L00_00326048(obj,*(char **)(obj+0x68)+12,0x80AAAAD2,0x80396464,0.061086535f);
            }
            i++; objects++;
        } while(i<ctx->count);
    }
    if(D_001A79F0==0) {
        if(ctx->state==0) {
            if(ctx->timer<2) func_002ED260(*(char **)((char *)ctx+0x70)+0x24000);
            func_00282C88(D_L00_001B24B0);
            D_L00_001B24B0[0]=-3.0f;
            ctx->objects[2][0x95]=255;
            func_L00_002E6CF0(func_L00_003618D8);
        } else if(ctx->state==2) {
            if(ctx->timer<2) func_002ED260(*(char **)((char *)ctx+0x70)+0x24000);
            func_00282C88(D_L00_001B24B0);
            ctx->objects[1][0x95]=255;
            func_L00_002E6CF0(func_L00_003618D8);
        }
    }
}
