#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L20_0043ECB0);

INCLUDE_ASM("asm/levels", func_L20_0043F318);

/* func_L20_0043F460 is defined in L20_0043ECB0-g8.c (it reaches a global through $gp). */

INCLUDE_ASM("asm/levels", func_L20_0043F570);

INCLUDE_ASM("asm/levels", func_L20_0043FC70);

INCLUDE_ASM("asm/levels", func_L20_0043FFF0);

INCLUDE_ASM("asm/levels", func_L20_00440198);

INCLUDE_ASM("asm/levels", func_L20_00440398);

extern void func_002A7750(int, int, int);

void func_L20_00440460(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);

    *(unsigned short *)(state + 0x178) = 0;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(int *)(arg0 + 0x98) = 0;
    *(unsigned short *)(arg0 + 0x34) = (*(unsigned short *)(arg0 + 0x34) | 0x41) & 0xEFFF;
    func_002A7750(arg0, 6, 3);
}

extern void func_00282CC0(char *, char *, char *);
extern void func_00282D30(char *, char *, float);
extern void func_002A8C28(char *, char *, int, int, float, float, float, int,
                          int);
extern int func_00282948(char *);
extern void func_0029F338(char *);

void func_L20_004404B8(char *arg0) {
    char *q = arg0 + 0x10;
    char *state = *(char **)(arg0 + 0x68);

    func_00282CC0(q, q, state);
    *(float *)(state + 8) = *(float *)(state + 8) - *(float *)(state + 0x14);
    func_00282D30(state, state, *(float *)(state + 0x10));
    func_002A8C28(*(char **)(arg0 + 0xB8), q, 0x10001, 0,
                  *(float *)(state + 0x18), 3.0f, 1.0f, 1, 0);
    if (func_00282948(state + 0x1C) != 0) {
        func_0029F338(arg0);
    }
}

extern void func_0027DDC0(int, int, int, int);

void func_L20_00440570(void) {
    func_0027DDC0(0, 0, 0, 0x80);
}

INCLUDE_ASM("asm/levels", func_L20_00440598);

INCLUDE_ASM("asm/levels", func_L20_00441CB8);

INCLUDE_ASM("asm/levels", func_L20_004421A8);

INCLUDE_ASM("asm/levels", func_L20_004424A0);

INCLUDE_ASM("asm/levels", func_L20_00442550);

extern void func_002A8C28(char *, char *, int, int, float, float, float, int,
                          int);

extern float func_00282E80(char *, char *);

extern void func_002B50E0(char *, char *, float);

extern void func_L01_00321178(char *, char *);

void func_L20_004427C8(char *arg0, char *arg1, float f) {
    char *state = *(char **)(arg0 + 0x68);

    func_002A8C28(arg0, arg0 + 0x10, 0x04010000, 0,
                  *(float *)(state + 0x50) + 0.5f, 50.0f, 1.0f, 1, 0);
    if (func_00282E80(arg0 + 0x10, arg1) <= *(float *)(state + 0x84)) {
        func_002B50E0(arg0, arg1, f);
    } else {
        func_L01_00321178(arg0, arg1);
    }
}

INCLUDE_ASM("asm/levels", func_L20_00442890);

INCLUDE_ASM("asm/levels", func_L20_004429F0);

extern int func_002A7820(int, int);

extern int func_L20_004433B0(char *);

extern int func_002A77E0(int);

void func_L20_00442C10(char *arg0) {
    char *state = *(char **)(arg0 + 0x68);
    int v = *(int *)(state + 0x17F4);
    int r;

    if (v == 2) goto other;
    if (v >= 3) goto other;
    if (v != 1) goto other;
    r = func_L20_004433B0(arg0);
    *(int *)(state + 0x17FC) = r;
    if (r == 0) {
        func_002A7750(arg0, 0xC, 4);
    } else {
        func_002A7750(arg0, 4, 2);
    }
    goto done;
other:
    r = func_002A77E0(2) + 2;
    *(int *)(state + 0x17F8) = r;
    *(int *)(state + 0x1810) = func_002A7820(0xB4, 0x168);
    func_002A7750(arg0, 3, 1);
done:
    *(int *)(state + 0x17F4) = *(unsigned char *)(arg0 + 0x95);
}

INCLUDE_ASM("asm/levels", func_L20_00442CD8);

INCLUDE_ASM("asm/levels", func_L20_00442E68);

extern int func_002ABB78(char *);

extern void func_L02_003A6F78(char *);

extern void func_L20_00449A78(char *);

void func_L20_00442F30(char *arg0) {
    if (func_002ABB78(arg0) != 0) {
        func_L02_003A6F78(arg0);
    } else if (*(short *)(arg0 + 0xAA) == 0x12D3) {
        func_L20_00449A78(arg0);
    }
}

INCLUDE_ASM("asm/levels", func_L20_00442F80);

INCLUDE_ASM("asm/levels", func_L20_004431E0);
/* func_L20_004431E0 reads a global through $gp: its near miss is in
   nonmatching/levels/func_L20_004431E0.c and belongs in the -G8 sibling. */

extern unsigned int D_L20_001B2F9C __attribute__((sda));
extern int func_L03_003CA138(char *);
INCLUDE_ASM("asm/levels", func_L20_004499E8);
extern int func_L20_00449AC8(char *);

extern void func_0039BDF0(char *, char *, int, int);

char *func_L20_00443328(char *arg0, char *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    unsigned int p = D_L20_001B2F9C + (*(int *)(state + 0x238) << 8);

    if (func_L20_00449AC8((char *)p) != 0) {
        func_L20_004499E8((char *)p, arg0, arg1);
        func_0039BDF0((char *)p, (char *)(p + 0x10), 0x168, 0);
        return (char *)p;
    }
    return 0;
}
INCLUDE_ASM("asm/levels", func_L20_004433B0);

extern int func_0033E808(char *, int);

extern int func_003BA3F8(char *);

extern int func_L20_00449AD8(char *);

int func_L20_00443508(char *arg0, char *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    short v;

    if (func_0033E808(arg1 + 0x10, *(int *)(state + 0x220)) == 0) {
        goto ret0;
    }
    v = *(short *)(arg1 + 0xAA);
    if (v < 0x1FF) {
        goto ret0;
    }
    if (v < 0x201) {
        goto other;
    }
    if (v != 0x12D3) {
        goto ret0;
    }
    if (func_L20_00449AC8(arg1) != 0) {
        goto ret0;
    }
    if (func_L20_00449AD8(arg1) != 0) {
        goto ret0;
    }
    return 1;
other:
    return func_003BA3F8(arg1) == 0;
ret0:
    return 0;
}

INCLUDE_ASM("asm/levels", func_L20_004435A0);

INCLUDE_ASM("asm/levels", func_L20_00443790);

INCLUDE_ASM("asm/levels", func_L20_00443908);

INCLUDE_ASM("asm/levels", func_L20_00443A10);

INCLUDE_ASM("asm/levels", func_L20_00443B60);

INCLUDE_ASM("asm/levels", func_L20_00443CF0);

INCLUDE_ASM("asm/levels", func_L20_00443D58);

INCLUDE_ASM("asm/levels", func_L20_00443DF0);

INCLUDE_ASM("asm/levels", func_L20_00444018);

/* func_L20_00444088 is defined in L20_00444088-g8.c (it reaches a global through
   $gp and needs its own -fno-schedule-insns). */

INCLUDE_ASM("asm/levels", func_L20_00444130);

INCLUDE_ASM("asm/levels", func_L20_00444138);
/* func_L20_00444138 reads a global through $gp: its near miss is in
   nonmatching/levels/func_L20_00444138.c and belongs in the -G8 sibling. */

INCLUDE_ASM("asm/levels", func_L20_004441D8);

INCLUDE_ASM("asm/levels", func_L20_004448A8);

extern float func_00283248(float, float);

extern float func_00283BE0(float, float);

void func_L20_00444AA8(char *arg0, float *arg1) {
    char *state = *(char **)(arg0 + 0x68);
    float f;

    f = func_00283248(arg1[0] - *(float *)(arg0 + 0x10),
                      arg1[1] - *(float *)(arg0 + 0x14));
    f = func_00283BE0(f, *(float *)(arg0 + 0xF8));
    if (f > 1.0471977f) {
        f = 1.0471977f;
    } else if (f < -1.0471977f) {
        f = -1.0471977f;
    }
    *(float *)(state + 0x2B8) = f;
}

INCLUDE_ASM("asm/levels", func_L20_00444B48);

extern void func_L00_002E6EF8(void (*)(void), int);

void func_L20_00444C68(int arg0) {
    func_002A7750(arg0, 0x12, -1);
    func_L00_002E6EF8(func_L20_00440570, arg0);
}

INCLUDE_ASM("asm/levels", func_L20_00444CA8);

INCLUDE_ASM("asm/levels", func_L20_004450D8);

INCLUDE_ASM("asm/levels", func_L20_004451C0);

INCLUDE_ASM("asm/levels", func_L20_00445738);

INCLUDE_ASM("asm/levels", func_L20_00445890);

/* The level's own jump table (the label of the data blob at 0x002B5980). */
extern void *jtbl_002B5980[];

extern int func_002B3868(char *);

extern int func_002B38B8(char *);

extern void func_002AA548(char *, char *, float, float, float, float);

extern unsigned char D_0019B2E8[];

/* The five behaviours the object's state byte selects; the table in the level's data
   holds the address of each. */
void func_L20_00446E70(char *arg0) {
    static void * volatile tab[] = { &&c0, &&c1, &&c2, &&c3, &&c4 };
    char *state;
    unsigned char sel;

    sel = *(unsigned char *)(arg0 + 0x20);
    state = *(char **)(arg0 + 0x68);
    if (sel >= 5) {
        goto end;
    }
    goto *jtbl_002B5980[sel];
c0:
    *(float *)(state + 0x10) = *(float *)(arg0 + 0x10);
    func_002A7750(arg0, 1, -1);
    goto out;
c1:
    if ((D_0019B2E8[((unsigned char *)arg0)[0xB0] + *(int *)0x001A79F0 * 16] & 0x80) == 0) {
        goto end;
    }
    func_002A7750(arg0, 2, -1);
    goto out;
c2:
    if (func_002B3868(arg0) != 0) {
        *(int *)(state + 0x14) = 0;
    }
    func_002AA548(arg0 + 0x10, state + 0x14,
                  *(float *)(state + 0x10) - *(float *)(state + 0x0),
                  *(float *)(state + 0x8), *(float *)(state + 0x8),
                  *(float *)(state + 0xC));
    goto out;
c3:
    if (func_002B38B8(arg0) == 0) {
        goto end;
    }
    func_002A7750(arg0, 4, -1);
    goto out;
c4:
    if (func_002B3868(arg0) != 0) {
        *(int *)(state + 0x14) = 0;
    }
    func_002AA548(arg0 + 0x10, state + 0x14, *(float *)(state + 0x10),
                  *(float *)(state + 0x8), *(float *)(state + 0x8),
                  *(float *)(state + 0xC));
    goto out;
end:
out:
    return;
}

INCLUDE_ASM("asm/levels", func_L20_00446FA8);

INCLUDE_ASM("asm/levels", func_L20_004473B0);

INCLUDE_ASM("asm/levels", func_L20_00447640);

INCLUDE_ASM("asm/levels", func_L20_00447748);

extern char D_L20_002AB980[];

extern void func_L04_003A21F8();

void func_L20_00447908(void) {
    func_L04_003A21F8(D_L20_002AB980, 6);
}

INCLUDE_ASM("asm/levels", func_L20_00447930);

INCLUDE_ASM("asm/levels", func_L20_00448858);

INCLUDE_ASM("asm/levels", func_L20_00448A18);
/* func_L20_00448A18 reaches four globals through $gp: its near miss is in
   nonmatching/levels/func_L20_00448A18.c and belongs in the -G8 sibling. */
