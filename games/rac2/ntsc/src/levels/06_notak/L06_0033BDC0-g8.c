#include "common.h"

/* The bodies that reach a global through $gp live in this -G8 sibling of
   L06_0033BDC0.c; the declarations carry the sda attribute so that cc1 emits
   the one-instruction $gp form the retail code has. */

extern void func_002A7750(void *, int, int);
extern int func_002A9640(void *, int, int);
extern void func_L00_0032A258(void *, void *, void *, void *, int, int, int, int,
                               int, int, int);
extern void func_L01_0031A278(char *, float *, float, float, float);
extern void func_L06_00402230(char *);
extern void func_0029F338(void *);
extern void func_002A7398();
extern float func_00283C80(float, float);
extern float func_00283B98(float, float);
extern float func_00283198(float);
extern void func_002E5CF8(int, int, char *);

extern float D_L06_001B0020 __attribute__((sda));
extern float D_L06_001B0028 __attribute__((sda));
extern float D_L06_001B0030 __attribute__((sda));
extern int D_L06_001B0050 __attribute__((sda));
extern float D_L06_001AFE40 __attribute__((sda));
extern char *D_L06_001B281C[] __attribute__((sda));

/* a moby's three states: idle, hit, and gone */
void func_L06_00402440(char *a0)
{
    char *s0 = a0;
    char *s1;
    float buf[4];

    s1 = *(char **)(s0 + 0x68);

    switch (*(unsigned char *)(s0 + 0x20)) {
    case 0:
        if (*(int *)(s1 + 4) != 0)
            *(unsigned short *)(s0 + 0x34) |= 0x8000;
        func_002A7750(s0, 1, -1);
        break;
    case 1:
        if (func_002A9640(s0, 0x4010000, 0) != 0) {
            if (*(int *)s1 == 0)
                func_002A7750(s0, 2, -1);
        }
        *(unsigned char *)(s0 + 0xA8) = 0xFF;
        break;
    case 2:
        func_L00_0032A258(s0, &D_L06_001B0030, &D_L06_001B0028, &D_L06_001B0020,
                          0xFFFF, 0, D_L06_001B0050, 6, 0xFFFF, 0xFF, 0);
        func_L01_0031A278(s0, buf, 1.0f, 1.0f, 3.0f);
        func_L06_00402230(s0);
        func_0029F338(s0);
        break;
    }
}

/* a moby that has just come alive: it takes its state, its position and the
   level's step, and it is retired when its angle has turned too far */
extern void func_L06_00400498(char *a0);
