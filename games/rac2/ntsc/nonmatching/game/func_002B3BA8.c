/* func_002B3BA8 (1152 bytes) - near miss: this C compiles to 1108 bytes.

   Everything else matches retail instruction for instruction (prologue, the four
   candidate searches over the player, the D_001B1CB0 / D_001C13E8 / D_001B1CF0 lists,
   the gp-relative loads in the delay slots, the frame and stack slots). Two things differ:

   1. Wall (docs/COMPILER-NOTES.md, "Quadword copy source shapes"): retail materialises the
      source pointer of three of the 128-bit copies into aim->position
      (`addiu $a1,$a2,0xD0; lq $v0,0($a1)`, `addiu $v1,$s0,0x10; lq $v0,0($v1)`, and both
      arms of the final player/target choice), while this cc1's CSE folds the offset into
      the lq (`lq $v0,0xD0($a0)`). Retail looks like an inline-asm qcopy helper. Because
      retail needs $a1 for that pointer, it keeps `index` in $t0 (`daddu $t0,$a1,$zero` at
      +0x4); here `index` stays in $a1. A plain TI copy, an inline helper, a separate
      pointer variable and volatile all fold the same way (measured).
   2. Path A's func_002AFD88 call is cross-jumped (jump2) into path B's identical call
      sequence here; retail keeps both sequences. Calling path A through an alias
      (`extern void func_002AFD88_1(...)` with the same prototype) stops the cross-jump:
      the body is then 1124 bytes and the only remaining differences are those of item 1.
   The origin copy (`lq $v0,0($a2)` / `lq $v0,0x10($s4)`) does fold in retail, as here. */

#include "include_asm.h"

extern int D_001A8CA0 __attribute__((sda));
extern int func_002B3B20(char *p, char *q, float f12, float f13);
extern float func_002B38C8(char *object, char *target);

typedef int Vec128_2B3BA8 __attribute__((mode(TI)));

typedef struct {
    Vec128_2B3BA8 position;
    char *target;
    int found;
    int unused;
    int kind;
} Aim_2B3BA8;

typedef struct {
    char pad0[0xD0];
    Vec128_2B3BA8 position;
    char pad1[0x2290 - 0xE0];
    char *moby;
    int state;
} Player_2B3BA8;

extern Player_2B3BA8 D_00189E20;
extern int D_001A8BB0 __attribute__((sda));
extern short D_001B1CB0[2] __attribute__((sda));
extern short D_001B1CF0[2] __attribute__((sda));
extern char *D_001B1B5C __attribute__((sda));
extern int D_001A8CD4 __attribute__((sda));
extern char *D_001C13E8[];
extern char *func_002B39A0(char *, int, int *);
extern float *func_002AB1F0(char *);
extern void func_002AFD88(float, float, float, char *, char *);

int func_002B3BA8(char *self, int index, Vec128_2B3BA8 *point, int force, float range, float height)
{
    float origin[4];
    int kind;
    Aim_2B3BA8 *aim;
    char *best;
    char *mob;
    float best_distance;
    int best_kind;
    int i;

    if ((*(unsigned short *)(self + 0x34) & 0x20) == 0)
        return 0;
    aim = *(Aim_2B3BA8 **)(*(char **)(self + 0x68) + 0xC);
    if (point != 0)
        *(Vec128_2B3BA8 *)origin = *point;
    else
        *(Vec128_2B3BA8 *)origin = *(Vec128_2B3BA8 *)(self + 0x10);
    aim->target = D_00189E20.moby;
    aim->position = D_00189E20.position;
    aim->kind = 0;
    aim->found = 0;
    if (force == 0) {
        if (D_001A8BB0 != 0)
            return 0;
        if (D_00189E20.state == 0x6F)
            return 0;
    }
    if (index != -1) {
        mob = func_002B39A0(self, index, &kind);
        if (mob != 0) {
            aim->target = mob;
            aim->found = 1;
            aim->kind = kind;
            if (mob != D_00189E20.moby) {
                float *info;
                aim->position = *(Vec128_2B3BA8 *)(mob + 0x10);
                info = func_002AB1F0(mob);
                if (info != 0) {
                    if (D_001A8CA0 != 0)
                        func_002AFD88(0.0f, 0.0f, info[4], mob, (char *)aim);
                    else
                        *(float *)((char *)aim + 8) += info[4];
                }
            }
        }
    } else {
        best_distance = 9999999.0f;
        best = 0;
        best_kind = 0;
        mob = D_00189E20.moby;
        if (func_002B3B20(mob, (char *)origin, range, height)) {
            float distance = func_002B38C8(self, mob);
            if (distance < best_distance) {
                best = mob;
                best_distance = distance;
                best_kind = 1;
            }
        }
        for (i = 1; i <= D_001B1CB0[0]; i++) {
            mob = D_001B1B5C + (D_001B1CB0[i] << 8);
            if (mob != 0 && *(short *)(mob + 0xAA) == 0xCB
                && *(unsigned char *)(mob + 0x20) != 0xFE
                && *(unsigned char *)(mob + 0x20) != 0xFD
                && *(unsigned char *)(mob + 0x20) == 3
                && func_002B3B20(mob, (char *)origin, range, height)) {
                float distance = func_002B38C8(self, mob);
                if (distance < best_distance) {
                    best = mob;
                    best_distance = distance;
                    best_kind = 2;
                }
            }
        }
        for (i = 0; i < D_001A8CD4; i++) {
            mob = D_001C13E8[i];
            if (func_002B3B20(mob, (char *)origin, range, height)) {
                float distance = func_002B38C8(self, mob);
                if (distance < best_distance) {
                    best = mob;
                    best_distance = distance;
                    best_kind = 3;
                }
            }
        }
        for (i = 1; i <= D_001B1CF0[0]; i++) {
            mob = D_001B1B5C + (D_001B1CF0[i] << 8);
            if (mob != 0 && *(short *)(mob + 0xAA) == 0xEB0
                && *(unsigned char *)(mob + 0x20) != 0xFE
                && *(unsigned char *)(mob + 0x20) != 0xFD
                && func_002B3B20(mob, (char *)origin, range, height)) {
                float distance = func_002B38C8(self, mob);
                if (distance < best_distance) {
                    best = mob;
                    best_distance = distance;
                    best_kind = 4;
                }
            }
        }
        if (best != 0) {
            aim->kind = best_kind;
            aim->found = 1;
            aim->target = best;
            if (best == D_00189E20.moby) {
                aim->position = D_00189E20.position;
            } else {
                float *info;
                aim->position = *(Vec128_2B3BA8 *)(best + 0x10);
                info = func_002AB1F0(best);
                if (info != 0) {
                    if (D_001A8CA0 != 0)
                        func_002AFD88(0.0f, 0.0f, info[4], best, (char *)aim);
                    else
                        *(float *)((char *)aim + 8) += info[4];
                }
            }
        }
    }
    return aim->found;
}
