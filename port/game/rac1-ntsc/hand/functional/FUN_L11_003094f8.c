/* Carried from the PAL decompilation's src/overlays/shared/vendor_002C99E0.c (matched C there) (games/rac1) through the version
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
/* A float vector with 4-byte alignment (no 128-bit member). */
typedef struct {
    f32 x;
    f32 y;
    f32 z;
    f32 w;
} Vec4f_c3094f8;
/* The quadword. Not called u128: source files declare their own u128,
   and a second typedef of one name is an error. */
typedef u32 VecQuad_c3094f8 __attribute__((mode(TI), aligned(16)));
typedef union {
    VecQuad_c3094f8 q;
    f32 f[4];
    s32 i[4];
} Vec4_c3094f8;
extern int func_001F9688(int);
extern void func_0020C828(void *);
extern float func_001F9B80(void *, void *);
extern float func_001F9B48(void *, void *);
extern void func_00214DB0(void *, float, float, float);
extern float func_L00_001FF8B0(float, float);
extern void func_L00_001FF500(void *, void *, float);
extern float func_002132A8(float, float);
extern void func_001F9A10(void *, void *, void *);
extern int func_L00_001EFC70(void *, void *, int, void *, void *);
extern int func_001F9740(int *);
struct PClass_c3094f8 {
    u8 pad[0x24];
    f32 scale;
};
struct PMoby_c3094f8 {
    Vec4f_c3094f8 bsphere;
    Vec4f_c3094f8 pos;
    u8 state;
    u8 pad21[3];
    struct PClass_c3094f8 *pclass;
    struct PMoby_c3094f8 *next;
    f32 scale;
    u8 pad30[0x10];
    Vec4f_c3094f8 rot;
    u8 pad50[0x28];
    u8 *pvars;
    u8 pad7C[0x44];
    Vec4f_c3094f8 rot_row0;
};
typedef struct {
    Vec4_c3094f8 step;
    Vec4_c3094f8 target_pos;
    void *collision;
    struct PMoby_c3094f8 *target;
    s32 timer;
    f32 speed;
    f32 turn_speed;
} Missile_c3094f8;
extern float func_00213308(void);
extern void func_001F9CF8(void *, void *, void *);
extern void func_00214890(void *, void *, void *, float);
extern void func_L00_0026CBB0(void *, void *, int, int, int, int, float);
extern unsigned char *func_L00_002712B8(void *, void *, int, int, int, int, float, float, float, float, float);
extern float func_L00_0025BE00(float *, float *, float, float, float, float);
extern void func_L00_0024F7C8(void *, int, void *);
extern void func_L00_0025E450(void *, void *, void *, float, float, int, int, int, float, float, float, float, int, float, int, int, int, int);
extern char D_L11_001B2A80[];
extern char D_L11_00174860[];
extern float D_0015ED70 MACRO_ADDR;
extern float D_0015ED6C MACRO_ADDR;



void FUN_L11_003094f8(struct PMoby_c3094f8 *moby) {
    Missile_c3094f8 *data = (Missile_c3094f8 *)moby->pvars;
    Vec4f_c3094f8 *pos;
    Vec4_c3094f8 previous_pos, spray, offset, velocity, random;
    float fraction;
    struct PMoby_c3094f8 *target;
    char *texture;

    qcopy(&previous_pos, &moby->pos);
    moby->scale += (moby->pclass->scale - moby->scale) * 0.1f;
    random.q = 0;
    random.f[0] = func_002132A8(-1.0f, 1.0f);
    random.f[1] = func_002132A8(-1.0f, 1.0f);
    random.f[2] = func_002132A8(-1.0f, 1.0f);
    velocity.q = random.q;
    func_L00_001FF500(&velocity, &velocity, func_002132A8(0.1f, 0.2f) * D_0015ED6C);
    func_L00_001FF500(&offset, &data->step, -func_002132A8(D_0015ED6C * 0.1f, D_0015ED6C));
    func_001F9A10(&velocity, &velocity, &offset);
    func_L00_002712B8(&moby->pos, &velocity, func_001F9688(6), 0x7f, 0xb0b0b0,
               3, 40000.0f, 1000.0f, 1.0f, -0.0004f, 0.0f);
    texture = D_L11_001B2A80;
    for (fraction = 0.0f; fraction < 1.0f; fraction += 0.33333334f) {
        unsigned char *p;
        func_L00_001FF500(&offset, &data->step, func_002132A8(fraction, fraction + 0.33333334f) * data->speed);
        func_001F9A10(&offset, &offset, &moby->pos);
        p = func_L00_002712B8(&offset, &velocity, func_001F9688(60), 0x7f, 0x606060, 3, 40000.0f,
                       1000.0f, 1.0f, -0.0002f, 0.0f);
        if (p != 0) {
            unsigned char *texture_pixels = *(unsigned char **)(texture + 0x5c);
            unsigned char texture_pixel = texture_pixels[0];
            p[3] = 0x44;
            p[2] = texture_pixel;
        }
        random.q = 0;
        random.f[2] = 0.02f;
        func_001F9CF8(&random, &random, &moby->rot_row0);
        func_00214890(&random, &random, &moby->rot_row0, func_00213308());
        func_L00_0026CBB0(&offset, &random, 0x4f007fff, 0x1fffffff, func_001F9688(5), 1, 20000.0f);
    }
    pos = &moby->pos;
    target = data->target;
    if (target != 0 && target->state != 0xfe && target->state != 0xfd &&
        func_001F9B48(&data->target_pos, &target->pos) < 3.0f) {
        moby->rot.z = func_L00_001FF8B0(data->target->pos.x - moby->pos.x, data->target->pos.y - moby->pos.y);
        moby->rot.y = -func_L00_001FF8B0(func_001F9B80(pos, &data->target->pos), data->target->pos.z - moby->pos.z);
        qcopy(&data->target_pos, &data->target->pos);
    } else {
        func_L00_0025BE00(&moby->rot.z, &data->turn_speed, moby->rot.z, D_0015ED70 * 251.32741f,
               D_0015ED70 * 251.32741f, D_0015ED6C * 2513.2742f);
        func_L00_0025BE00(&moby->rot.y, &data->turn_speed, moby->rot.y, D_0015ED70 * 251.32741f,
               D_0015ED70 * 251.32741f, D_0015ED6C * 2513.2742f);
        data->target = 0;
    }
    func_00214DB0(&data->step, data->speed, moby->rot.z, -moby->rot.y);
    func_001F9A10(pos, pos, &data->step);
    if (moby->pos.x < 2.0f || 1021.0f < moby->pos.x || moby->pos.y < 2.0f || 1021.0f < moby->pos.y ||
        moby->pos.z < 2.0f || 1021.0f < moby->pos.z) {
        func_0020C828(moby);
        return;
    }
    func_L00_0024F7C8(moby, 1, &spray);
    if (func_L00_001EFC70(&previous_pos, &spray, 0, data->collision, 0)) {
        func_L00_0025E450(moby, data, D_L11_00174860, 2.0f, 10.0f, 10, 3, 0x10, 4.0f, 2.0f, 9.0f, 1.0f,
                -1, 20.0f, 0, 1, -1, 0);
        func_0020C828(moby);
        return;
    }
    if (func_001F9740(&data->timer))
        func_0020C828(moby);
}
