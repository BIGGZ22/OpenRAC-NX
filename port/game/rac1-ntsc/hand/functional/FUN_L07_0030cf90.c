/* Carried from the PAL decompilation's src/overlays/l07_umbris/vendor_002CE470.c (matched C there) (games/rac1) through the version
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
extern float func_001F9B80(void *, void *);
extern void func_L00_0028D918(int);
extern int func_0022DA68(int, int, int);
extern void func_001F9A10(void *, void *, void *);
extern char D_0013E533[];
typedef int Q30E370_c30cf90 __attribute__((mode(TI)));
extern void func_001F9CF8(void *, void *, void *);
extern int D_0015ED84 MACRO_ADDR;
extern char D_0014161B[];
extern int D_L07_00184528[];
extern char *D_L07_0015FFD8 MACRO_ADDR;
extern float D_0015ED60 MACRO_ADDR;
typedef struct {
    float origin_x;
    float origin_y;
    float origin_z;
    short sound_slot;
    signed char trigger_sound;
    signed char arrival_sound;
    int target_index;
    short flag_index;
    short target_state;
    int target_state_word;
    float radius;
} Vars30E370_c30cf90;



void FUN_L07_0030cf90(unsigned char *moby) {
    Vars30E370_c30cf90 *vars = *(Vars30E370_c30cf90 **)(moby + 0x78);
    float origin[4];
    float rotated[4] __attribute__((aligned(16)));
    float step[4] __attribute__((aligned(16)));
    unsigned char *target;
    short voice;
    float distance;
    float step_size;

    origin[0] = vars->origin_x;
    origin[1] = vars->origin_y;
    origin[2] = vars->origin_z;
    origin[3] = 0.0f;
    if (moby[0xB0] != 0xff && ((unsigned char *)(D_0014161B + 0xAA35))[moby[0xB0] + (D_0015ED84 << 4)] == 0xff)
        vars->target_index = -1;

    switch (moby[0x20]) {
    case 0:
    {
        float *pos = (float *)(moby + 0x10);
        vars->origin_x = pos[0];
        vars->origin_y = pos[1];
        vars->origin_z = pos[2];
        vars->sound_slot = -1;
        if (vars->flag_index != -1)
            D_L07_00184528[vars->flag_index] = 0;
        moby[0x20] = 1;
        break;
    }
    case 1:
        if (vars->target_index != -1) {
            target = (unsigned char *)(D_L07_0015FFD8 + vars->target_index * 0x100);
            if (target != 0 && target[0x20] != 0xfe && target[0x20] != 0xfd &&
                target[0x20] != vars->target_state)
                return;
        }
        if (vars->trigger_sound != -1)
            func_0022DA68(vars->trigger_sound, 0, (int)moby);
        moby[0x20] = 2;
        break;
    case 2:
        if (vars->target_index != -1 &&
            (target = (unsigned char *)(D_L07_0015FFD8 + vars->target_index * 0x100)) != 0 &&
            target[0x20] != 0xfe && target[0x20] != 0xfd &&
            target[0x20] == vars->target_state_word) {
            func_0022DA68(vars->trigger_sound, 0, (int)moby);
            moby[0x20] = 4;
            break;
        }
        distance = func_001F9B80((float *)(moby + 0x10), origin);
        if (vars->radius < distance) {
            if (vars->arrival_sound != -1)
                func_0022DA68(vars->arrival_sound, 0, (int)moby);
            if (vars->flag_index != -1)
                D_L07_00184528[vars->flag_index] = 1;
            moby[0x20] = 3;
        } else {
            *(Q30E370_c30cf90 *)step = 0;
            step[1] = -(D_0015ED60 * 0.01f);
            *(Q30E370_c30cf90 *)rotated = *(Q30E370_c30cf90 *)step;
            if (*(short *)(moby + 0xA6) == 0x3f5 || *(short *)(moby + 0xA6) == 0x429)
                rotated[1] = -rotated[1];
            func_001F9CF8(step, rotated, moby + 0xC0);
            func_001F9A10(moby + 0x10, moby + 0x10, step);
            return;
        }
        break;
    case 3:
        if (vars->target_index != -1) {
            target = (unsigned char *)(D_L07_0015FFD8 + vars->target_index * 0x100);
            if (target != 0 && target[0x20] != 0xfe && target[0x20] != 0xfd) {
                if (target[0x20] != vars->target_state_word)
                    return;
                if (vars->trigger_sound != -1)
                    func_0022DA68(vars->trigger_sound, 0, (int)moby);
                moby[0x20] = 4;
            }
        }
        break;
    case 4:
        if (vars->target_index == -1 ||
            (target = (unsigned char *)(D_L07_0015FFD8 + vars->target_index * 0x100)) == 0 ||
            target[0x20] == 0xfe || target[0x20] == 0xfd ||
            target[0x20] == vars->target_state) {
            func_0022DA68(vars->trigger_sound, 0, (int)moby);
            moby[0x20] = 2;
            return;
        }
        distance = func_001F9B80((float *)(moby + 0x10), origin);
        step_size = D_0015ED60 * 0.01f;
        if (distance <= step_size) {
            float *p = (float *)(moby + 0x10);
            p[0] = vars->origin_x;
            p[1] = vars->origin_y;
            p[2] = vars->origin_z;
            p[3] = 0.0f;
            voice = vars->sound_slot;
            if (voice != -1) {
                unsigned char *slot = (unsigned char *)D_0013E533 + 0x1D + voice * 0x70;
                if (*(unsigned char **)(slot + 0x88) == moby && slot[0x74] != 0)
                    func_L00_0028D918(voice);
            }
            vars->sound_slot = -1;
            if (vars->arrival_sound != -1)
                func_0022DA68(vars->arrival_sound, 0, (int)moby);
            if (vars->flag_index != -1)
                D_L07_00184528[vars->flag_index] = 0;
            moby[0x20] = 1;
            break;
        }
        *(Q30E370_c30cf90 *)step = 0;
        step[1] = step_size;
        *(Q30E370_c30cf90 *)rotated = *(Q30E370_c30cf90 *)step;
        if (*(short *)(moby + 0xA6) == 0x3f5 || *(short *)(moby + 0xA6) == 0x429)
            rotated[1] = -rotated[1];
        func_001F9CF8(step, rotated, moby + 0xC0);
        func_001F9A10(moby + 0x10, moby + 0x10, step);
        break;
    case 5:
        break;
    }
}
