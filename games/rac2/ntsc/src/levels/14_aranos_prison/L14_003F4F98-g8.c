#include "include_asm.h"

extern int D_L14_001B17D8 __attribute__((sda));
extern int D_L14_001B17DC;
extern int D_001A72E0;

extern int func_00283D38(int a, int b, float c);
extern void func_385CB8(unsigned int reg, unsigned long value);
extern void func_3030C0(int, int, int, int, int, int);

/* Plays one sound effect derived from the owner's state block and then queues the level jingle. */
void func_L14_00427250(char *a0)
{
    char *v0;
    int s0;

    v0 = *(char **)(a0 + 0x68);
    s0 = func_00283D38(D_L14_001B17D8, D_L14_001B17DC, *(float *)v0);
    func_385CB8(0x42, 0x8000000044UL);
    func_3030C0(0, 0, 600, 500, s0, 0);
    func_385CB8(0x4E, 0x1000000UL | (D_001A72E0 >> 13));
}

extern float D_L14_001B17B8 __attribute__((sda));
extern float D_L14_001B17BC __attribute__((sda));

extern char D_00189EA0[];

extern int func_00282948(void *table);

extern void *func_L00_002D70A0(int size);

extern void func_L00_002D7140(void *block);

/* Rewinds the step timer, then respawns the effect the first live slot names, or frees it. */
INCLUDE_ASM("asm/levels", func_L14_00426E60);

extern float D_L14_001B16B0 __attribute__((sda));
extern int func_002A9640(char *a0, int a1, int a2);
extern void func_002B0ED8(char *a0, char *a1, int a2);
extern void func_002AC728(char *a0, char *a1, float f12, float f13, int a2);
extern void func_002A6C58(char *a0, int a1, int a2);
extern void func_0029F338(char *a0);

/* Resets the moby's scale when its model says so, then lets it fall or die. */
void func_L14_00423240(char *a0)
{
    char *s0 = a0;
    char *s1;
    char *m;
    char *v;

    s1 = *(char **)(s0 + 0x68);
    v = (char *)func_002A9640(a0, 0x10000, 1);
    if (v != 0) {
        m = *(char **)(v + 0x20);
        if (m != 0 && *(short *)(m + 0xAA) == 0xB69) {
            if (*(unsigned char *)(s0 + 0x31) == 0)
                *(float *)(v + 0x2C) = 0.0f;
            else
                *(float *)(v + 0x2C) = 1.0f;
        }
    }
    func_002B0ED8(s0, s1 + 0x60, 0);
    if (*(float *)(s1 + 0x20) <= 0.0f) {
        func_002AC728(s0, s0 + 0x10, 1.0f, 13.0f, -1);
        func_002A6C58(s0, 0, 3);
        func_0029F338(s0);
    } else if (*(float *)(s0 + 0x18) < D_L14_001B16B0) {
        func_002A6C58(s0, 0, 3);
        func_0029F338(s0);
    }
}

extern float D_L14_001B1678 __attribute__((sda));
extern float D_L14_001B167C __attribute__((sda));
extern int D_L14_001B1660 __attribute__((sda));
extern int D_L14_001B1668 __attribute__((sda));
extern int D_L14_001B1690 __attribute__((sda));
extern int D_L14_001B1694 __attribute__((sda));
extern float D_L14_001B1698 __attribute__((sda));
extern float D_L14_001B169C __attribute__((sda));
extern float D_L14_001B16A0 __attribute__((sda));
extern float D_L14_001B16A4 __attribute__((sda));
extern char D_L14_002594F0[][0x280];
extern void func_L14_004229A0(char *, char *, int, int, int, float, float, float);

void func_L14_004228C0(char *object) {
    char *state = *(char **)(object + 0x68);

    D_L14_001B1678 -= D_L14_001B167C;
    if (D_L14_001B1678 <= -8.0f)
        D_L14_001B1678 += 8.0f;
    func_L14_004229A0(object, D_L14_002594F0[*(int *)(state + 0x38)],
        D_L14_001B1660, D_L14_001B1668, *(int *)(state + 0x2C) - 2,
        *(float *)(state + 0x40), *(float *)(state + 0x44), D_L14_001B16A0);
    func_L14_004229A0(object, D_L14_002594F0[*(int *)(state + 0x38)] + 0x140,
        D_L14_001B1690, D_L14_001B1694, *(int *)(state + 0x2C),
        D_L14_001B1698, D_L14_001B169C, D_L14_001B16A4);
}

extern int D_L14_001B259C __attribute__((sda));

extern int func_L00_00324120(char *self);
extern void func_L00_003294F0(int table, int mode);
extern void func_002A7750(char *self, int state, int mode);
extern int func_002B3868(char *self);
extern void func_002E5CF8(int a, int b, char *self);
extern float func_00283B98(float a, float b);
extern float func_00283198(float a);
extern int func_00283D38(int a, int b, float c);

/* Steps the owner's state block, then blends the two colours its state asks for. */
void func_L14_00409B20(char *self)
{
    char *v = *(char **)(self + 0x68);
    int c1 = 0x808C8C8C;
    int c2 = 0x80282828;
    int idx;
    float f;
    int r;

    switch (*(unsigned char *)(self + 0x20)) {
    case 0:
        if (func_L00_00324120(self) != 0) {
            idx = *(int *)(v + 0);
            if (idx != -1) {
                func_L00_003294F0(D_L14_001B259C + (idx << 8), 1);
            }
            func_002A7750(self, 1, -1);
        }
        break;
    case 1:
        if (func_002B3868(self) != 0) {
            func_002E5CF8(0, 0, self);
        }
        c1 = 0x80468CDC;
        c2 = 0x801E78AA;
        break;
    }
    f = func_00283B98(*(float *)(v + 4), 0.10762865f);
    *(float *)(v + 4) = f;
    r = func_00283D38(c1, c2, func_00283198(f) * 0.5f + 0.5f);
    *(int *)(self + 0x78) = func_00283D38(*(int *)(self + 0x78), r, 0.15f);
}
