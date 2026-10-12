

#ifndef RAC2_T_U8_FE9770B1E
#define RAC2_T_U8_FE9770B1E
typedef unsigned char u8_Fe9770b1e;
#endif

#ifndef RAC2_T_S32_FE9770B1E
#define RAC2_T_S32_FE9770B1E
typedef int s32_Fe9770b1e;
#endif

#ifndef RAC2_T_F32_FE9770B1E
#define RAC2_T_F32_FE9770B1E
typedef float f32_Fe9770b1e;
#endif

/* Resident shared table reached through $gp (small-data symbol). */
extern f32_Fe9770b1e D_L04_001AE0B8[2] __attribute__((sda));

extern void *func_00336E40_1(void *);

extern void *func_00336DA8_1(s32_Fe9770b1e, void *);

extern void func_0033ABD8_1(u8_Fe9770b1e *, void *, f32_Fe9770b1e *);

extern void *func_00288FB8_1(s32_Fe9770b1e);

extern void func_0033B050_1(u8_Fe9770b1e *, void *, void *, void *);

extern void func_0033B030_1(u8_Fe9770b1e *, f32_Fe9770b1e, f32_Fe9770b1e, f32_Fe9770b1e, f32_Fe9770b1e, f32_Fe9770b1e, f32_Fe9770b1e);

extern void func_0033B0A8_1(u8_Fe9770b1e *, f32_Fe9770b1e);

extern void func_L00_00432448(u8_Fe9770b1e *, s32_Fe9770b1e);

void func_0033C9F0(u8_Fe9770b1e *self, void *arg) {
    u8_Fe9770b1e *sub;
    void *first, *second, *third;
    *(void **)self = arg;
    if (arg != 0) {
        void *t = func_00336E40_1(arg);
        void *p = func_00336DA8_1(16, t);
        *(void **)(self + 684) = p;
        ((s32_Fe9770b1e *)p)[1] = 0;
        ((s32_Fe9770b1e *)p)[2] = 0;
        ((s32_Fe9770b1e *)p)[3] = 0;
        ((s32_Fe9770b1e *)p)[0] = 0;
    }
    sub = self + 8;
    ((f32_Fe9770b1e *)*(void **)(self + 684))[0] = 255.0f;
    ((f32_Fe9770b1e *)*(void **)(self + 684))[1] = 195.0f;
    func_0033ABD8_1(sub, arg, D_L04_001AE0B8);
    first = func_00288FB8_1(11667);
    second = func_00288FB8_1(11598);
    third = func_00288FB8_1(11599);
    func_0033B050_1(sub, first, second, third);
    func_0033B030_1(sub, 0.0f, -143.0f, 0.0f, 114.0f, 0.0f, 140.0f);
    func_0033B0A8_1(sub, 0.2f);
    *(s32_Fe9770b1e *)(self + 680) = 0;
    func_L00_00432448(self, 0);
}
