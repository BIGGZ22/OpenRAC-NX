/* Carried from the PAL decompilation's src/overlays/shared/vendor_002D7C00.c (matched C there) (games/rac1) through the version
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
extern float func_001F9B48(void *, void *);
extern void func_001F9A28(void *, void *, void *);
extern int func_001F9770(void *);
extern char D_0013E533[];
extern void func_0022DA68(int, int, void *);
/* NON_MATCHING FUN_L15_002e5958 -- src/overlays/shared/vendor_002D7C00.c
 * Best so far: SIZE ours 1288 / retail 1300, checked 2026-10-09.
 * Not built into anything: the retail assembly stays in the source file
 * until a candidate is EXACT (docs/NONMATCHING.md). Start from this one.
 * What the last attempts found:
 *   Underwater ring update (moby class 1209): an approach test, then a five-state switch on moby[0x20] with a jump
 *   Remaining differences are scheduling and argument placement: a `daddu $a0,$s1` before the func_L00_002502F0 ca
 */
extern float func_001F9AF0(void *a);
extern void func_001FA2D8(void *, void *);
extern void func_001F9D20(void *, void *, void *);
extern void func_L00_0024F7C8(void *, int, void *);
extern int func_L00_002591D0(int *, int, int, int);
extern int func_L00_002592B8(int *, int, int, int);
extern void func_L00_002502F0(void *, int, int, int);
extern void func_L01_0026E0E0(int list, int state);
extern char D_L15_001BB830[];
extern int D_0015ED84 MACRO_ADDR;
extern int D_L15_001BAAD0[];
extern char D_0014161B[];
typedef struct {
    short timer;
    short delay;
    short triggered;
    short flash_timer;
    unsigned char flash_color;
    unsigned char initialized;
    unsigned char child_count;
    unsigned char notified_count;
    int frame_count;
    unsigned char *parent;
    float distance;
    short restart_timer;
    short pad26;
    int enabled;
} RingVars_6D48_c2e5958;



/* Underwater ring update: approach check, then the ring's state machine. */
void FUN_L15_002e5958(unsigned char *moby) {
    RingVars_6D48_c2e5958 *vars = *(RingVars_6D48_c2e5958 **)(moby + 0x78);
    unsigned char delta[16];
    unsigned char matrix[64];
    int child;
    unsigned short save_id;
    unsigned char *save_byte;
    int first;
    int second;
    int divisor;
    int near_parent = 0;

    if ((unsigned char)(moby[0x20] - 1) < 2) {
        func_001F9A28(delta, moby + 0x10, D_0013E533 + 0xE9D);
        if (func_001F9AF0(delta) < vars->distance) {
            func_001FA2D8(matrix, moby + 0xC0);
            func_001F9D20(delta, delta, matrix);
            if (*(float *)(delta + 4) < 0.2f) near_parent = 1;
        }
    }
    switch (moby[0x20]) {
    case 0:
        if (vars->initialized == 0) {
            func_L00_0024F7C8(moby, 0, delta);
            vars->distance = func_001F9B48(moby + 0x10, delta);
            vars->notified_count = 0;
            vars->parent = moby;
            func_L00_002591D0(&child, moby[0x21], 0, 0);
            while (child != 0) {
                if ((unsigned char *)child != moby && *(short *)(child + 0xA6) == 0x4b9) {
                    RingVars_6D48_c2e5958 *other = *(RingVars_6D48_c2e5958 **)(child + 0x78);
                    other->parent = moby;
                    other->initialized = 1;
                    other->distance = vars->distance;
                }
                vars->notified_count++;
                func_L00_002592B8(&child, child, 0, 0);
            }
        }
        vars->timer = 0;
        vars->delay = 0;
        vars->child_count = 0;
        vars->triggered = 0;
        vars->flash_timer = 0;
        func_L00_002502F0(moby, 0, 0, 0);
        moby[0x20] = 1;
        break;
    case 1:
    case 2:
        save_id = *(unsigned short *)(moby + 0xB2);
        save_byte = (unsigned char *)D_L15_001BB830;
        save_byte -= -(short)save_id;
        if (save_byte[0x454] != 0 ||
            ((((int)*(unsigned int *)(((((short)save_id >> 5) << 2) + (D_0015ED84 << 8)) + (unsigned char *)(D_0014161B + 0xAB75))) >> (save_id & 31)) & 1) != 0) {
            if (vars->enabled != 0) {
                func_L01_0026E0E0(moby[0x21], 4);
                vars->timer = vars->restart_timer;
                moby[0x20] = 4;
                break;
            }
        }
        if (vars->triggered == 0 && near_parent) {
            unsigned char *parent = vars->parent;
            RingVars_6D48_c2e5958 *parent_vars = *(RingVars_6D48_c2e5958 **)(parent + 0x78);
            if (parent[0x20] == 1) {
                vars->child_count = 0;
                parent[0x20] = 2;
                parent_vars->timer = parent_vars->frame_count < 1 ? 900 : func_001F9688(vars->frame_count);
            }
            parent_vars->child_count++;
            func_0022DA68(3, 0x31, (int)moby);
            func_L00_002502F0(moby, 0, 0xff, 0);
            vars->triggered = 1;
        }
        if (moby[0x20] == 2) {
            if (vars->child_count >= vars->notified_count) {
                func_L01_0026E0E0(moby[0x21], 4);
                ((int (*)(int, int, int))func_0022DA68)(1, 0x31, (int)moby);
                vars->timer = vars->restart_timer;
                moby[0x20] = 4;
                break;
            } else {
                if (func_001F9770(&vars->timer) != 0) {
                    vars->child_count = 0;
                    vars->triggered = 0;
                    func_0022DA68(2, 0x31, (int)moby);
                    func_L01_0026E0E0(moby[0x21], 3);
                } else if (func_001F9770(&vars->delay) != 0) {
                    func_0022DA68(0, 0x31, (int)moby);
                    first = func_001F9688(0x2d);
                    second = func_001F9688(5);
                    divisor = func_001F9688(0x28);
                    if (first < second + vars->timer / divisor)
                        vars->delay = func_001F9688(0x2d);
                    else {
                        int delay = func_001F9688(5);
                        int delay_divisor = func_001F9688(0x28);
                        delay_divisor = delay + vars->timer / delay_divisor;
                        vars->delay = delay_divisor;
                    }
                }
            }
        }
        break;
    case 3:
        func_L00_002502F0(moby, 0, 0, 0);
        vars->child_count = 0;
        vars->triggered = 0;
        moby[0x20] = 1;
        break;
    case 4: {
        unsigned short save_id;
        unsigned int *bits;
        if (func_001F9770(&vars->flash_timer) != 0) {
            vars->flash_timer = func_001F9688(0x14);
            if (vars->flash_color != 0) {
                func_L00_002502F0(moby, 0, 0xff, 0);
                vars->flash_color = 0;
            } else {
                func_L00_002502F0(moby, 0xff, 0xff, 0xff);
                vars->flash_color = 1;
            }
        }
        save_id = *(unsigned short *)(moby + 0xB2);
        bits = (unsigned int *)(((((short)save_id >> 5) << 2) + (D_0015ED84 << 8)) + (unsigned char *)(D_0014161B + 0xAB75));
        *bits |= 1U << (save_id & 31);
        save_id = *(unsigned short *)(moby + 0xB2);
        D_L15_001BAAD0[(short)save_id >> 5] |= 1U << (save_id & 31);
        break;
    }
    }
}
