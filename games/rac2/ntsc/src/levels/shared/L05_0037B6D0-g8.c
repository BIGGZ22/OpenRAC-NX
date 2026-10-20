#include "include_asm.h"

/* The gate's word at $gp-0x2B14 is a small index into a table of live sub-objects. */
extern int D_L05_001AC4DC __attribute__((sda));

extern char D_L05_00221D10[];
extern char D_L05_00225DD0[];

extern char *func_379CA0(char *object);
extern int func_37CDB0(char *object);

extern int func_L05_003CFC28(int);
extern float func_002A7878(float, float);
extern float func_002A7940(void);

/* The reach table the object's id indexes, at $gp+0xB78. */
extern float D_L05_001AFB68 __attribute__((sda));

extern int D_001A79F0;

extern char *func_002ED688(char *object);

extern void func_L05_00379FA0(char *, int, int, int, float, float, float, float, int, int, int);

extern int func_002A77E0(int);

extern void func_002E5DA0(int, int, char *, int);

/* Decides whether the object may take its next step: the gate must be open, the looked-up
 * sub-object must exist, its handle must match the one the gate's table names, it must not be
 * already counting, its two counters must be ordered, and its state must be neither 3 nor 0xD.
 * The last test writes 0 rather than branching. */
int func_L05_0037B888(char *a0)
{
    char *object = a0;
    char *sub = func_379CA0(a0);
    char *gate = D_L05_00221D10;
    int ready = 1;

    if (*(int *)(gate + 0x2C) == 1)
        return 0;
    if (sub == 0)
        return 0;
    if (*(short *)(object + 0xAA) != *(short *)(((char **)D_L05_00225DD0)[D_L05_001AC4DC] + 0xAA))
        return 0;
    if (*(int *)(sub + 0x118) == 1)
        return 0;
    if (*(int *)(gate + 0xC) >= *(int *)(gate + 0x20))
        return 0;
    if (func_37CDB0(object) == 3)
        return 0;
    if (func_37CDB0(object) == 0xD)
        ready = 0;
    return ready;
}

/* Gives the object a random reach, three random angles and a random cone, and writes the
 * scaled results into its state. */
void func_L05_003CF9C8(char *obj)
{
    char *s0 = obj;
    char *s1 = *(char **)(obj + 0x68);
    float f0;
    float f1;
    float f20;
    float f21;
    float f22;
    int idx;

    *(unsigned char *)(s0 + 0x30) = 0xFF;
    *(unsigned short *)(s0 + 0x32) = 0x1FF;
    idx = func_L05_003CFC28(*(short *)(s0 + 0xAA));
    f21 = 0.0f;
    f20 = 0.01745329424738884f;
    f22 = 1.0f;
    f1 = *(float *)(s0 + 0x2C) * *(float *)((char *)&D_L05_001AFB68 + 4 * idx);
    *(float *)(s0 + 0x2C) = f1;
    f0 = func_002A7940();
    *(float *)(s0 + 0xF0) = f0;
    f0 = func_002A7940();
    *(float *)(s0 + 0xF4) = f0;
    f0 = func_002A7940();
    *(float *)(s0 + 0xF8) = f0;
    f0 = func_002A7878(f21, 0.5f) * f20;
    *(float *)(s1 + 0x0) = f0;
    f0 = func_002A7878(f21, f22) * f20;
    *(float *)(s1 + 0x4) = f0;
    f0 = func_002A7878(f21, 2.0f) * f20;
    *(float *)(s1 + 0x8) = f0;
    *(float *)(s1 + 0xC) = f22;
}

/* Spawns the object's effect once its guards clear, then runs the timer and marks the run.
 * The two words of the effect's parameters are the same in both calls, so the compiler
 * loads the constant once and copies it into the two argument registers. */
void func_L05_003D0420(char *obj)
{
    char *s1 = *(char **)(obj + 0x68);
    char *r;

    r = func_002ED688(obj);
    if (r == 0)
        return;
    if (*(int *)(r + 0x120) == 0)
        return;
    if (*(int *)(s1 + 0x1B8) != 0)
        return;
    if (*(unsigned char *)(obj + 0x20) == 1) {
        if (D_001A79F0 == 0xF)
            func_L05_00379FA0(obj, 0x60008000, 1, 0, 0.6f, 2.0f, 1.0f, 1.0f, 0, -1, 0);
        else
            func_L05_00379FA0(obj, 0x60008000, 1, 0, 1.0f, 2.0f, 1.0f, 1.0f, 0, -1, 0);
    }
    if (func_002A77E0(3) == 0)
        func_002E5DA0(0, 0, obj, 0x957);
    *(int *)(s1 + 0x1B8) = 0x14;
}
