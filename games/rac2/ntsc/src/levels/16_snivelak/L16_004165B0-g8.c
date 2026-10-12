#include "include_asm.h"

typedef unsigned char u8;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

extern s32 D_0018B650 __attribute__((nosda));
extern s32 D_L16_001B16C0 __attribute__((sda));
extern void func_002C4368(s32);
extern void func_00433698(s32);

/* clears the level state word and reseeds the two engine subsystems */
void func_L16_00416E88(void) {
    D_0018B650 = 0;
    D_L16_001B16C0 = 0;
    func_002C4368(0);
    func_00433698(3);
}

extern f32 D_L16_001B1E90 __attribute__((sda));
extern f32 D_L16_001B1E94 __attribute__((sda));
extern void func_0031C698(char *, char *, char *, f32, f32, f32);

/* submits the sub-object at 0x68 with the level's angle scale */
void func_L16_0041C948(char *arg0) {
    char *p = arg0;
    char *st = *(char **)(p + 0x68);

    func_0031C698(p, st + 4, st, D_L16_001B1E90, D_L16_001B1E94 * 0.000290888245f, -1.0f);
}

extern char D_00189EA0[];
extern s32 D_L16_001B1A80 __attribute__((sda));
extern void func_002E7968(char *, s32 *, char *);
extern void func_002E7488(char *, char *, char *);

/* installs the two level matrices on the shared render object */
void func_L16_00416E30(char *arg0) {
    char *obj = D_00189EA0;

    func_002E7968(obj, &D_L16_001B1A80, arg0 + 0xC0);
    func_002E7488(obj, obj, arg0 + 0x10);
    *(f32 *)(obj + 0x18) = *(f32 *)(arg0 + 0xF8);
}

extern f32 D_L16_001B1C28 __attribute__((sda));
extern f32 D_L16_001B1C2C __attribute__((sda));
extern f32 D_L16_001B1C30 __attribute__((sda));
extern f32 D_L16_001B1C34 __attribute__((sda));

/* initialises the level object's scale/limit table */
void func_L16_00418218(char *p) {
    f32 a = D_L16_001B1C28;
    f32 b = D_L16_001B1C2C;
    f32 c = D_L16_001B1C30;
    f32 d = D_L16_001B1C34;

    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 84) = 0;
    *(float *)(p + 88) = 1.0f;
    *(int *)(p + 92) = 0;
    *(int *)(p + 96) = 0;
    *(float *)(p + 100) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = a;
    *(float *)(p + 24) = b;
    *(long long *)(p + 128) = 0x0000FF9000000260LL;
    *(float *)(p + 40) = a;
    *(float *)(p + 4) = c;
    *(float *)(p + 36) = d;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(float *)(p + 56) = b;
    *(float *)(p + 20) = c;
    *(float *)(p + 52) = d;
    *(float *)(p + 60) = 1.0f;
}
