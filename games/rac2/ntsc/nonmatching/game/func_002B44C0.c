/* func_002B44C0 (1248 bytes) - near miss: this C compiles to 1192 bytes.

   A sway/spring update for an attached object: target vector (argument, or the player's
   position at D_00189E20+0x80 when D_00189E20+0x33C is this object and +0x340 >= 2),
   per-axis spring/damping over three axes, angle limits, then the object's matrix at
   +0xC0 and position at +0x10 are rebuilt. The control flow, the struct layout, the stack
   frame (0x140, locals at 0x00-0xBF in this order), the hoisted address pseudos that are
   spilled to 0xC8-0xEC, both 3-step loops and the clamps come out the same shape.

   What differs:
   1. Wall (docs/COMPILER-NOTES.md, "Quadword copy source shapes"): the two 128-bit copies
      at the top keep their pointers in registers in retail
      (`addiu $s1,$a0,0x10; lq $v0,0($s1); sq $v0,0($sp)` and
      `addiu $a0,$v0,0xF0; lq $v0,0($a0); addiu $v1,$sp,0x10; sq $v0,0($v1)`), as does the
      copy of sway->offset into `work` (through two spilled pointers). This cc1's CSE folds
      them (`lq $v0,0x10($a0)`), with or without a separate pointer variable.
   2. As a consequence, retail keeps `self + 0x10` in $s1 for the two func_00282CF0 calls of
      the target branch and keeps the `target` argument in $s3; here the pointer is rebuilt
      in each branch and the callee-saved registers and spill slots shift
      (retail order 0xC8 self+0xC0, 0xCC work, 0xD0 matrix, 0xD4 self+0x10,
      0xD8 sway+0x10, 0xDC moved, 0xE0 sway+0x20, 0xE4 shift, 0xE8 angles, 0xEC delta walk).
   Untested ideas once item 1 is solvable: the order of the hoisted address pseudos decides the
   spill slots, and the x/y/z clamps compile with different branch-likely shapes in retail. */

#include "include_asm.h"

typedef int Vec128_2B44C0 __attribute__((mode(TI)));

typedef struct {
    Vec128_2B44C0 anchor;
    Vec128_2B44C0 offset;
    Vec128_2B44C0 rotation;
    float position[3];
    float velocity[3];
    float spring[3];
    float damping[3];
    float limit[3];
    float scale[3];
    float kick[3];
    float phase[3];
    float speed[3];
    float amplitude[3];
    int unused;
    int timer;
} Sway_2B44C0;

typedef struct {
    char pad0[0x80];
    Vec128_2B44C0 position;
    char pad1[0x33C - 0x90];
    char *target;
    int state;
} Player_2B44C0;

extern Player_2B44C0 D_00189E20;
extern void func_00115484(void *, int, int);
extern void func_00282CF0(void *, void *, void *);
extern void func_00282CC0(void *, void *, void *);
extern void func_002AFDD8(char *, void *, void *, void *);
extern int func_00282948(void *);
extern float func_00283B98(float, float);
extern float func_00283198(float);
extern void func_002833F0(void *, void *);
extern void func_00283738(void *, void *, void *);
extern void func_00283098(void *, void *, void *);
extern void func_002B0158(char *, void *, void *, float);
extern char *func_002AD0B0(char *);
extern void func_002AD6F0(char *, void *, void *, void *);

void func_002B44C0(char *self, Sway_2B44C0 *sway, float *target)
{
    float old_position[4];
    float old_rotation[4];
    float push[4];
    float delta[4];
    float angles[4];
    float work[4];
    float matrix[12];
    float moved[4];
    float shift[4];
    float step[4];
    char *body;
    float roll;
    int i;

    *(Vec128_2B44C0 *)old_position = *(Vec128_2B44C0 *)(self + 0x10);
    *(Vec128_2B44C0 *)old_rotation = *(Vec128_2B44C0 *)(self + 0xF0);
    func_00115484(push, 0, 16);
    if (target != 0 || (D_00189E20.target == self && D_00189E20.state >= 2)) {
        if (target != 0)
            func_00282CF0(delta, target, self + 0x10);
        else
            func_00282CF0(delta, &D_00189E20.position, self + 0x10);
        func_002AFDD8(self, delta, delta, 0);
        func_00282CF0(delta, delta, &sway->offset);
        push[0] = -delta[1] * sway->scale[0];
        push[1] = delta[0] * sway->scale[1];
        push[2] = -sway->scale[2];
        if (sway->timer == 0) {
            for (i = 0; i < 3; i++)
                sway->velocity[i] += push[i] * sway->kick[i];
            sway->timer = 10;
        }
    } else {
        func_00282948(&sway->timer);
    }
    for (i = 0; i < 3; i++) {
        sway->velocity[i] += sway->spring[i] * (push[i] - sway->position[i]);
        sway->velocity[i] -= sway->damping[i] * sway->velocity[i];
        sway->position[i] += sway->velocity[i];
        sway->phase[i] = func_00283B98(sway->phase[i], sway->speed[i]);
        delta[i] = sway->amplitude[i] * func_00283198(sway->phase[i]);
    }
    *(Vec128_2B44C0 *)work = 0;
    work[0] = sway->position[0] + delta[0];
    work[1] = sway->position[1] + delta[1];
    *(Vec128_2B44C0 *)angles = *(Vec128_2B44C0 *)work;
    if (sway->limit[0] < angles[0])
        angles[0] = sway->limit[0];
    else if (angles[0] < -sway->limit[0])
        angles[0] = -sway->limit[0];
    if (sway->limit[1] < angles[1])
        angles[1] = sway->limit[1];
    else if (angles[1] < -sway->limit[1])
        angles[1] = -sway->limit[1];
    func_002833F0(matrix, angles);
    func_002833F0(self + 0xC0, &sway->rotation);
    func_00283738(self + 0xC0, self + 0xC0, matrix);
    *(Vec128_2B44C0 *)work = sway->offset;
    work[2] += delta[2];
    func_00283098(moved, work, self + 0xC0);
    func_00282CC0(moved, moved, self + 0x10);
    func_00282CF0(shift, &sway->anchor, moved);
    func_00282CC0(self + 0x10, self + 0x10, shift);
    roll = -sway->position[2];
    if (sway->limit[2] < roll)
        roll = sway->limit[2];
    else if (roll < -sway->limit[2])
        roll = -sway->limit[2];
    func_002B0158(self, self + 0x10, self + 0x10, roll);
    body = func_002AD0B0(self);
    if (body != 0) {
        func_00282CF0(step, self + 0x10, old_position);
        func_002AD6F0(body, step, old_rotation, self + 0xF0);
    }
}
