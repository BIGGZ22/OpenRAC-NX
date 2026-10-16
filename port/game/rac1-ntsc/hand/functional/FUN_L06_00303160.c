/* Carried from the PAL decompilation's src/overlays/l06_blarg/vendor_002FE5D0.c (matched C there) (games/rac1) through the version
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
extern int func_001F9688(int);
extern float func_001F9DC8(float);
extern void func_001F9A40(void *, void *, void *, float);
extern float func_001FA5C8(float, float);
extern void func_001F99F8(void *);
extern float func_L00_001FF8B0(float, float);
extern float func_001F9B80(void *, void *);
extern void func_L00_002EA9D8(void *);
extern void func_L00_002EAA30(void *);
extern float func_001FA580(float, float);
extern float D_0015ED70  MACRO_ADDR;
extern float D_0015ED6C  MACRO_ADDR;
typedef struct {
    float x, y, z, w;
} __attribute__((aligned(16))) PathPoint_303160_c303160;
typedef struct {
    int n;
    int pad[3];
    PathPoint_303160_c303160 p[1];
} Path_303160_c303160;
struct PathRide_303160_c303160 {
    char pad0[0x60];
    int path_a[5];
    int path_b[5];
    int path_c[5];
    char pad1[0x14];
    float sway;
    float sway_speed;
    char pad2[0x0C];
    float scale_start;
    float scale_end;
    char pad3[4];
    int sound_slot[9];
    float progress;
};
extern float func_00213ED8(float *p, float target, float maxstep);
extern int func_001FA6D0(float);
extern float func_L00_0025BE00(float *p, float *v, float a, float b, float c, float d);
extern int func_0022DBA0(int, int, void *);
extern void func_L06_00303748(float, struct Moby_04FA8 *, void *, void *, int);
extern void func_L06_00303B68(struct Moby_04FA8 *);
extern char D_L06_001B0C30[] ;
extern char *D_L06_001600EC  MACRO_ADDR;


/* Moves the moby along three paths and plays the sound marked on the path points. */
int FUN_L06_00303160(char *moby, int path) {
    float rotation[4], position[4];
    struct PathRide_303160_c303160 *ride = *(struct PathRide_303160_c303160 **)(moby + 0x78);
    Path_303160_c303160 *first_path = ((Path_303160_c303160 **)D_L06_001B0C30)[ride->path_a[path]];
    Path_303160_c303160 *middle_path = ((Path_303160_c303160 **)D_L06_001B0C30)[ride->path_b[path]];
    Path_303160_c303160 *third_path = ((Path_303160_c303160 **)D_L06_001B0C30)[ride->path_c[path]];
    float time, first_index_f, first_fraction, third_fraction, angle, rate;
    int first_index, middle_index, third_index, marker, prev_marker;
    char *sound;

    func_00213ED8(&ride->progress, 1.0f, 1.0f / (float)func_001F9688(0x4B0));
    time = ride->progress * (float)(first_path->n - 1);
    first_index = func_001FA6D0(time);
    first_index_f = (float)first_index;
    first_fraction = time - first_index_f;
    middle_index = func_001FA6D0(ride->progress * (float)(middle_path->n - 1));
    time = ride->progress * (float)(third_path->n - 1);
    third_fraction = time - first_index_f;
    third_index = func_001FA6D0(time);
    func_001F9A40(position, &first_path->p[first_index], &first_path->p[first_index + 1], first_fraction);
    func_001F9A40(moby + 0x10, &third_path->p[third_index], &third_path->p[third_index + 1], third_fraction);
    func_L00_002EA9D8(position);
    if (third_index > 0) {
        angle = func_L00_001FF8B0(third_path->p[third_index].x - third_path->p[third_index - 1].x, third_path->p[third_index].y - third_path->p[third_index - 1].y);
        *(float *)(moby + 0x48) = func_001FA580(
            func_001FA5C8(func_L00_001FF8B0(third_path->p[third_index + 1].x - third_path->p[third_index].x, third_path->p[third_index + 1].y - third_path->p[third_index].y), angle) * third_fraction,
            angle);
        angle = -func_L00_001FF8B0(func_001F9B80(&third_path->p[third_index - 1], &third_path->p[third_index]), third_path->p[third_index].z - third_path->p[third_index - 1].z);
        *(float *)(moby + 0x44) = func_001FA580(
            func_001FA5C8(-func_L00_001FF8B0(func_001F9B80(&third_path->p[third_index], &third_path->p[third_index + 1]), third_path->p[third_index + 1].z - third_path->p[third_index].z),
                           angle) * third_fraction,
            angle);
        *(float *)(moby + 0x44) = *(float *)(moby + 0x44) * func_001F9DC8(ride->sway);
        *(float *)(moby + 0x48) = func_001FA580(*(float *)(moby + 0x48), ride->sway);
    } else {
        *(float *)(moby + 0x44) =
            -func_L00_001FF8B0(func_001F9B80(&third_path->p[0], &third_path->p[1]), third_path->p[1].z - third_path->p[0].z);
        *(float *)(moby + 0x48) = func_L00_001FF8B0(third_path->p[1].x - third_path->p[0].x, third_path->p[1].y - third_path->p[0].y);
    }
    if (first_index > 0 && middle_index > 0) {
        func_001F99F8(rotation);
        angle = func_L00_001FF8B0(middle_path->p[middle_index].x - position[0], middle_path->p[middle_index].y - position[1]);
        rotation[2] = func_001FA580(
            func_001FA5C8(func_L00_001FF8B0(middle_path->p[middle_index + 1].x - position[0], middle_path->p[middle_index + 1].y - position[1]), angle) * third_fraction, angle);
        angle = -func_L00_001FF8B0(func_001F9B80(position, &middle_path->p[middle_index]), middle_path->p[middle_index].z - position[2]);
        rotation[1] = func_001FA580(
            func_001FA5C8(-func_L00_001FF8B0(func_001F9B80(position, &middle_path->p[middle_index + 1]), middle_path->p[middle_index + 1].z - position[2]), angle) * third_fraction,
            angle);
        func_L00_002EAA30(rotation);
    }
    *(float *)(moby + 0x2C) =
        *(float *)(*(char **)(moby + 0x24) + 0x24) * (ride->scale_start - (ride->scale_start - ride->scale_end) * ride->progress);
    if (ride->progress > 0.8f) {
        rate = D_0015ED70 * 12.566371f;
        func_L00_0025BE00(&ride->sway, &ride->sway_speed, 3.1415927f, rate, rate, D_0015ED6C * 6.2831855f);
    }
    marker = func_001FA6D0(first_path->p[first_index].w);
    if (first_index != 0)
        prev_marker = func_001FA6D0(first_path->p[first_index - 1].w);
    else
        prev_marker = -1;
    if (marker >= 0 || prev_marker >= 0) {
        first_path->p[first_index].w = -1.0f;
        first_path->p[first_index - 1].w = -1.0f;
        if (marker < 8 && prev_marker < 8) {
            func_0022DBA0(0, 0, moby);
            sound = D_L06_001600EC + (ride->sound_slot[marker] << 7);
            func_L06_00303748(6.0f, (struct Moby_04FA8 *)moby, sound + 0x30, sound, 6);
        } else if (marker == 10 || prev_marker == 10) {
            func_0022DBA0(1, 0, moby);
            func_L06_00303B68((struct Moby_04FA8 *)moby);
        }
    }
    return ride->progress == 1.0f;
}
