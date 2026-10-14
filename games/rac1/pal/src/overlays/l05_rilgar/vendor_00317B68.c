/* Kept in a separate translation unit: this rotor reads the pool pointer with full addressing,
 * while vendor_0030EB68.c also has small-data declarations of that symbol. */
#include "common.h"
extern float func_001FA790(float, float);
extern float func_001FA748(float, float);

typedef struct {
    char pad[0xBC];
    unsigned char flag;
    char pad2[0x43];
} RotorPoolMoby17B68;
extern RotorPoolMoby17B68 *D_L05_00160098_pool __asm__("D_L05_00160098") MACRO_ADDR;
extern float D_0015EE70 MACRO_ADDR;
extern float D_0015EE6C MACRO_ADDR;

typedef struct {
    int flag;
    float velocity;
    int target;
    float angle;
} RotorData17B68;
typedef struct {
    char pad[0x20];
    unsigned char state;
    char pad1[0x13];
    unsigned short flags;
    char pad2[0x12];
    float rot;
    char pad3[0x2C];
    RotorData17B68 *data;
} RotorMoby17B68;

extern int func_0022ED80_i(int, int, void *) __asm__("func_0022ED80");
extern float ease_rotation_317B68(float *, float, float *, float, float, float) __asm__("func_L00_0025CE58");
/* Updates a triggered rotor: initialize its angle, wait for the target, then ease its rotation. */
void func_L05_00317B68(RotorMoby17B68 *moby) {
    RotorData17B68 *p = moby->data;
    float f;
    switch (moby->state) {
    case 0:
        if (p->flag != 0) {
            moby->flags |= 0x8000;
        }
        moby->state = 1;
        p->angle = moby->rot;
        if (p->flag != 0) {
            moby->rot = func_001FA790(moby->rot, 1.5707964f);
        } else {
            moby->rot = func_001FA748(moby->rot, 1.5707964f);
        }
        /* fallthrough */
    case 1:
        if (D_L05_00160098_pool[p->target].flag != 0) {
            func_0022ED80_i(0, 0, moby);
            moby->state = 2;
        }
        break;
    case 2:
        if (p->flag != 0) {
            f = func_001FA790(p->angle, 0.6981317f);
        } else {
            f = func_001FA748(p->angle, 0.6981317f);
        }
        ease_rotation_317B68(&moby->rot, f, &p->velocity, D_0015EE70 * 12.566371f, D_0015EE70 * 12.566371f, D_0015EE6C * 12.566371f);
        break;
    }
}
