#include "common.h"

/* func_L23_004160B0 is a unit of its own: it reads $gp (hence -g8), and it is built with -g,
   whose line label in front of the division makes the assembler pad it with retail's two nops
   (the effect func_L23_003BF9B0 and func_L23_004402D8 use). The division stays in a statement of
   its own for that label.
   Measured on this body:
   - the three float constants of the loop are locals set at the top of the `if` block, in the
     order 90.0f, 1/60, pi/180: loop.c hoists invariants in the order it finds them, and retail
     has them in that order ahead of the loop. As literals, the pi/180 factor (used first) comes
     out ahead of the other two; set at the top of the loop body instead, the 90.0f local is
     replaced by its constant in both calls (gcse's constant propagation) and is not hoisted;
   - `0.017453294f` is the spelling of retail's 0x3C8EFA36;
   - `hit = 0` follows `if (f()) g();`, or the call's delay slot takes it;
   - `base` is read inside the loop, which gives the hoisted address pair and the
     `lwc1 $f13,0x158($s6)` in the call's delay slot;
   - func_00273730 is declared with its int 0xFF argument last, for the `addiu $t3` in the delay
     slot of its call;
   - D_L23_001B13CC is read as a variable (retail compares with its value), not written 0.0f. */

extern float D_L23_001B13C8 __attribute__((sda));

extern float D_L23_001B13CC __attribute__((sda));

extern char *D_L23_001B1484 __attribute__((sda));

extern char D_L23_001BA500[];

extern char *func_002A9640(char *, int, int);

extern void func_00282978(char *);

extern int func_L23_0041B948(void);

extern void func_L23_0041E570(void);

extern void func_002AB800(char *, char *);

extern void func_002AB8C0(char *, char *);

extern void func_L23_0041C578(char *);

extern void func_L23_0041BE70(char *);

extern float func_002A78D8(float, float);

extern float func_00283B98(float, float);

extern float func_002A7878(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern char *func_00273730(char *, int, int, int, char *, char *, float *, float, float, int);

extern void func_00310C88(void);

/* Updates the hit state of the object, and when its mask word still has bits set, spawns one
   particle per set bit (at most ten, fewer the fuller the value at +0x20 is), each launched at
   a random angle and speed, and clears the bit. */
void func_L23_004160B0(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    char *hit;
    int n;
    int i;
    float vec[3];

    hit = func_002A9640(arg0, 0x30000, 1);
    *(unsigned char *)(arg0 + 0xA8) = 0xFF;
    func_00282978(state + 0x26);
    if (hit != 0) {
        if (*(short *)(state + 0x26) != 0 || (D_L23_001B1484 != 0 && func_L23_0041B948() != 0)) {
            if (func_L23_0041B948() != 0)
                func_L23_0041E570();
            hit = 0;
        }
    }
    if (hit != 0) {
        *(unsigned char *)(state + 0x147) = 0xFF;
        func_002AB800(arg0, state + 0x140);
        func_L23_0041C578(*(char **)(hit + 0x20));
        if (*(float *)(state + 0x20) <= D_L23_001B13CC) {
            func_L23_0041BE70(hit);
            *(float *)(state + 0x20) = D_L23_001B13CC;
        } else {
            *(float *)(state + 0x20) = *(float *)(state + 0x20) - *(float *)(hit + 0x2C);
        }
        if (*(float *)(state + 0x20) <= D_L23_001B13CC)
            *(float *)(state + 0x20) = D_L23_001B13CC;
        if (*(short *)(*(char **)(hit + 0x20) + 0xAA) != 0x118F)
            *(short *)(state + 0x26) = 0x14;
        if (*(unsigned short *)(arg0 + 0x7C) != 0) {
            float f;

            f = *(float *)(state + 0x20) / D_L23_001B13C8;
            n = 10 - (int)(f * 10.0f);
            if (*(float *)(state + 0x20) <= D_L23_001B13CC)
                n = 10;
            for (i = 0; i < n; i++) {
                int mask = *(unsigned short *)(arg0 + 0x7C);

                if ((mask >> i) & 1) {
                    float angle, speed;
                    char *p;
                    char *base = D_L23_001BA500;
                    float spread = 90.0f;
                    float per_tick = 0.016666668f;
                    float to_rad = 0.017453294f;

                    *(unsigned short *)(arg0 + 0x7C) = mask & ~(1 << i);
                    angle = func_00283B98(func_002A78D8(spread, 120.0f) * to_rad,
                                          *(float *)(base + 0x158));
                    speed = func_002A7878(2.0f, 4.0f) * per_tick;
                    vec[0] = func_00283180(angle) * speed;
                    vec[1] = func_00283198(angle) * speed;
                    vec[2] = 0.0f;
                    vec[2] = func_002A7878(5.0f, 8.0f) * per_tick;
                    p = func_00273730(arg0, 1, *(short *)(arg0 + 0xAA), i, arg0 + 0x10, arg0 + 0xF0,
                                      vec, spread, 15.0f, 0xFF);
                    if (p != 0)
                        *(void (**)(void))(p + 0x64) = func_00310C88;
                }
            }
        }
    }
    func_002AB8C0(arg0, state + 0x140);
}
