#include "include_asm.h"

typedef struct YeedilEffectState {
    char pad00[0x30];
    int mode;
    int frame;
    char pad38[0x154];
    char *first;
    int unused190;
    char *second;
} YeedilEffectState;
extern YeedilEffectState D_L20_001BDCC0;
extern int D_L20_001A8F00[] __attribute__((sda));
extern void func_0032C700(void *, int, int, int, int, int, int, int,
                         int, int, int, int, float, float);
extern void func_0029FCC0(void *, int, void *);
extern float func_002A7878(float, float);
extern float func_002A7940(void);
extern void func_002AF000(void *, float, float, float);
extern int func_002A77E0(int);
extern void func_002BE278(void *, void *, int, float);

void func_L20_0039AA98(unsigned char *object)
{
    float position[4], velocity[4];
    float speed, heading, tilt;
    YeedilEffectState *state;

    switch (object[0x20]) {
    case 0:
        object[0x30] = 0xFF;
        object[0x20] = 1;
        break;
    case 1:
        if (D_L20_001A8F00[0] != 2)
            break;
        state = &D_L20_001BDCC0;
        if (state->mode != 7)
            break;
        if (state->frame == 157)
            func_0032C700(state->second + 0x10, 1, 40, 20, 30, 40, 50, 1,
                          1, 1, 0, 0, 1.5f, 20.0f);
        if ((unsigned int)(state->frame - 171) < 149) {
            func_0029FCC0(state->first, 0, position);
            position[2] += 0.03f;
            speed = func_002A7878(0.028333336114883423f, 0.05000000447034836f);
            heading = func_002A7940();
            tilt = func_002A7878(0.523598849773407f, 2.6179940700531006f);
            func_002AF000(velocity, speed, heading, tilt);
            if (func_002A77E0(2))
                func_002BE278(position, velocity, 0, func_002A7878(1470.0f, 7350.0f));
        }
        break;
    }
}

/* func_L20_002DEBE0 is defined in L20_002DEBE0-g8.c (it reaches data through $gp). */

/* func_L20_00310E38 is defined in L20_002DEBE0-g8.c (it reaches data through $gp). */

extern unsigned char D_L20_001DBA60[];

extern void *D_L20_001DB100[];

extern void func_0029F390(void *);

extern void func_002A0F48(void *);

void func_L20_00330A68(void *arg0, int arg1)
{
    unsigned char *p;
    unsigned char *r;
    unsigned char *s;
    int q;

    *(short *)(arg0 + 0xAA) = (short)arg1;
    *(unsigned char *)(arg0 + 0x22) = D_L20_001DBA60[(short)arg1];
    *(void **)(arg0 + 0x24) = D_L20_001DB100[*(unsigned char *)(arg0 + 0x22)];
    func_0029F390(arg0);
    p = *(unsigned char **)(arg0 + 0x24);
    *(unsigned char *)(arg0 + 0x62) = p[0xE];
    *(float *)(arg0 + 0x2C) = *(float *)(p + 0x24);
    *(int *)(arg0 + 0x98) = *(int *)(p + 0x10);
    *(unsigned char *)(arg0 + 0x61) = 0xFF;
    if (p) {
        q = *(int *)(p + 0x40);
        if (q) {
            *(unsigned short *)(arg0 + 0x34) |= 0x10;
            *(int *)(arg0 + 0x78) = *(int *)(p + 0x40);
        }
        r = *(unsigned char **)(arg0 + 0x24);
        if (r[0xF]) {
            *(unsigned char *)(arg0 + 0x6F) = 0x18;
            *(unsigned short *)(arg0 + 0x34) |= 0x400;
            *(int *)(arg0 + 0x70) = 0;
            *(int *)(arg0 + 0x74) = 0;
            *(unsigned char *)(arg0 + 0xBD) = 0;
        }
        s = *(unsigned char * volatile *)(arg0 + 0x24);
        if (s[6]) {
            *(unsigned char *)(arg0 + 0x63) = 0x18;
        }
    }
    func_002A0F48(arg0);
}

/* Catalogue defect: the row's first two words (0x0036AE68 `sw $t3,0x40($v1)` /
   `nop`) sit after the previous function's `jr $ra` (0x0036AE5C) and are
   unreachable; the next eight words (0x0036AE70 `addiu $sp,$sp,0x20` /
   `nop`) have the shape of the four-byte stub rows this level's splitter
   cuts out of a function (131 of them, e.g. func_L00_002B2C50 at
   0x00308688), so the real body starts at 0x0036AE78 and is 272 bytes: a
   leaf function with no frame, no saved register and `jr $ra` at 0x0036AF80.
   It reaches data through $gp twice (0x0036AF0C and 0x0036AF58, both
   `lw $a0,-0x4800($gp)` -> 0x001AA7F0), so it belongs in the -G8 unit once
   the row is split. */
INCLUDE_ASM("asm/levels", func_L20_0036AE68);

extern void func_L00_002F3478(void);

extern void func_002B49E0(int a, int b, int c, int d, int e);

extern int D_L20_001FFE40[];

extern int D_L00_001A8FDC __attribute__((sda));

extern int D_L20_00294F98;

void func_L20_0036E720(void)
{
    int *base;
    int *other;

    func_L00_002F3478();
    base = D_L20_001FFE40;
    other = &D_L20_00294F98;
    *(int *)(base + 0x40) = (int)other;
    *(int *)(base + 0x41) = 0xB;
    D_L00_001A8FDC = 1;
    func_002B49E0(3, 1, 0xB, 0, 0);
}

/* func_L20_0039BDF0 is a near miss: nonmatching/levels/func_L20_0039BDF0.c */

INCLUDE_ASM("asm/levels", func_L20_0039BDF0);

/* func_L20_0039BB18 is defined in L20_002DEBE0-g8.c (it reaches data through $gp). */

/* func_L20_0039BEC8 is defined in L20_002DEBE0-g8.c (it reaches data through $gp). */
