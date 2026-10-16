/* Carried from the PAL decompilation's nonmatching/functional/func_L06_003006F8.c (games/rac1) through the version
 * map (../../README.md): functional C, not matched C. Names and addresses are this version's. */
#include "types.h"
#include "qcopy.h"
#include "qzero.h"
#ifndef MACRO_ADDR
#define MACRO_ADDR
#endif
#ifndef NOT_SDA
#define NOT_SDA
#endif
#ifndef SDATA
#define SDATA(symbol) __asm__(#symbol)
#endif
/* What the PAL file this was written in declares before it (its unit, and its own file). */
typedef struct {
    u8 pad0[0x18];
    u8 unk18;
    u8 unk19;
    s16 oclass;
    u8 pad1C[0x14];
} ShotInfo_c2ff2c8;
/* functional: medium. Per-frame update of a chain of 16 points (a whip/tentacle trail) hanging off a moby: advances a countdown, then each point is pulled toward the anchor at a speed (halved in vehicle mode 1), kept at a height by its alignment to the moby direction, collision-tested, and finally each adjacent pair that is solid enough spawns a hit; ends by queueing the draw callback and running the follow-up routine. Statically checked only. */
extern int f_f2i_03006F8(float) __asm__("func_001FA6D0");
extern int f_timer_03006F8(int *) __asm__("func_001F9740");
extern void f_sub_03006F8(void *, void *, void *) __asm__("func_001F9A28");
extern void f_norm_03006F8(void *, void *, float) __asm__("func_L00_001FF500");
extern void f_add_03006F8(void *, void *, void *) __asm__("func_001F9A10");
extern float f_dot_03006F8(void *, void *) __asm__("func_001F9AB0");
extern int f_hit_03006F8(void *, void *, int, void *, void *) __asm__("func_L00_001EFC70");
extern void f_approach_03006F8(float *, float, float) __asm__("func_00213ED8");
extern float f_cos_03006F8(float) __asm__("func_001F9DC8");
extern float f_sin_03006F8(float) __asm__("func_001F9DE0");
extern void f_shot_03006F8(void *, void *, int, void *, float) __asm__("func_L00_00259888");
extern void f_enqueue_03006F8(void *, void *) __asm__("func_001F4600");
extern void func_L06_002FF680(char *);
extern void func_L06_002FF978(char *);
extern unsigned char D_0013E633_03006F8[] __asm__("D_0013E533");
extern float D_0015EE6C_03006F8 __asm__("D_0015ED6C") MACRO_ADDR;
extern float D_L06_00162060_03006F8 __asm__("D_L06_00161FD0") MACRO_ADDR;
typedef u32 u128_3006F8_c2ff2c8 __attribute__((mode(TI), aligned(16)));

void func_L06_003006F8_x(char *moby) __asm__("FUN_L06_002ff2c8");


void func_L06_003006F8_x(char *moby) {
    char *data = *(char **)(moby + 0x78);
    int frames;
    int i;
    int next;
    int offset;
    float step;
    float alignment;
    float point[4];
    float relative[4];
    ShotInfo_c2ff2c8 shot;

    frames = f_f2i_03006F8(*(float *)(data + 0x21C));
    f_timer_03006F8(&frames);
    *(float *)(data + 0x21C) = (float)frames;
    if (frames == 0)
        return;
    if (frames == 20) {
        char *weight = data;
        int remaining = 15;
        do {
            float *value = (float *)(weight + 0x23C);
            if (*value >= 1.0f)
                *value = 0.98f;
            remaining--;
            weight += 0x10;
        } while (remaining >= 0);
    }
    i = 0;
    offset = 0;
    {
        char *entryc = data;
        float *entry;
        float *relative_ptr = relative;
        do {
            if (D_0013E633_03006F8[0x2EC1] == 1)
                step = D_L06_00162060_03006F8 * D_0015EE6C_03006F8 * 0.5f;
            else
                step = D_L06_00162060_03006F8 * D_0015EE6C_03006F8;
            entry = (float *)(entryc + 0x230);
            f_sub_03006F8(point, entry, data + 0x210);
            point[2] = 0.0f;
            f_norm_03006F8(point, point, step);
            f_add_03006F8(point, entry, point);
            f_sub_03006F8(relative_ptr, point, moby + 0x10);
            alignment = f_dot_03006F8(relative_ptr, data + 0x220);
            if (alignment > 0.4f)
                point[2] -= D_0015EE6C_03006F8 + D_0015EE6C_03006F8;
            else if (alignment < 0.3f)
                point[2] += D_0015EE6C_03006F8 + D_0015EE6C_03006F8;
            if (*(float *)(entryc + 0x23C) >= 1.0f && f_hit_03006F8(data + offset + 0x230, point, 2, 0, 0) != 0)
                point[3] = 0.98f;
            else if (point[3] < 1.0f)
                f_approach_03006F8(&point[3], 0.0f, D_0015EE6C_03006F8 + D_0015EE6C_03006F8);
            *(u128_3006F8_c2ff2c8 *)entry = *(u128_3006F8_c2ff2c8 *)point;
            i++;
            entryc += 0x10;
            offset = i * 16;
        } while (i < 16);
    }
    for (i = 0; i < 15; i = next) {
        char *entry = data + i * 16;
        char *next_entry;
        offset = i * 16;
        next = i + 1;
        if (*(float *)(entry + 0x23C) >= 0.6f &&
            (next_entry = data + next * 16, *(float *)(next_entry + 0x23C) >= 0.6f)) {
            point[0] = f_cos_03006F8(*(float *)(moby + 0x48));
            point[1] = f_sin_03006F8(*(float *)(moby + 0x48));
            point[2] = 1.0f;
            point[3] = 5627.925f;
            f_shot_03006F8(&shot, moby,
                           (*(float *)(entry + 0x23C) >= 1.0f && *(float *)(next_entry + 0x23C) >= 1.0f) ? 0x10001 : 0x10000,
                           point, 1.0f);
            shot.unk18 = 0;
            shot.unk19 = 1;
            shot.oclass = *(short *)(moby + 0xA6);
            f_hit_03006F8(data + offset + 0x230, data + offset + 0x240, 0, moby, &shot);
        }
    }
    f_enqueue_03006F8((void *)func_L06_002FF680, moby);
    func_L06_002FF978(moby);
}
