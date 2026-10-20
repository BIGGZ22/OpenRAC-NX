#include "include_asm.h"

extern void func_002E37B0(void);

extern void func_002E3548(void);

extern void func_002E3820(void);

extern void func_002FC560(unsigned int, unsigned long);

extern void func_L02_00363D78(void);

extern int D_001A79F0;

extern int D_001A72E0;

extern char D_L02_00280A10[];

extern int D_L02_001B33D0;

/* func_L02_003CB090 needs a $gp member and an absolute one; the measured
   92-of-96-byte candidate is recorded in nonmatching/levels/func_L02_003CB090.c
   (residue: branch-likely + which register gets the absolute address). */

extern int func_002ACBC8(void *point, int index);

extern void func_00282CF0(float *out, const float *a, const float *b);

extern void func_00283098(float *out, const float *a, const float *b);

extern void func_L02_002DCEA0(int a0, float a1);

extern void func_L00_002B9D60(int a0, int a1, int a2, int a3);

extern char D_00189EA0[];

extern char D_L02_001BA500[];

extern char *D_L02_001B368C;

extern float D_L02_001B2FE0 __attribute__((sda));


void func_L02_00300738(void) {
    func_002E37B0();
    D_L02_001B33D0 = (int)D_L02_00280A10;

    if (D_001A79F0 == 2)
        func_L02_00363D78();
    else
        func_002E3548();
    func_002E3820();
    func_002FC560(0x47, 0x5360B);
    func_002FC560(0x4E, 0x01000000 | (D_001A72E0 >> 13));
}

/* Flags the actor's target when its record is on screen, and refreshes it afterwards. */
void func_L02_003D02F0(char *arg0) {
    char *p = *(char **)(arg0 + 0x68);
    float v[4];

    if (*(int *)(p + 0xDC) == 0) {
        if (func_002ACBC8(D_00189EA0, *(int *)(p + 0xC4)) != 0) {
            func_00282CF0(v, (float *)D_L02_001BA500,
                          (float *)(D_L02_001B368C + (*(int *)(p + 0xC4) << 7) + 0x30));
            func_00283098(v, v, (float *)(D_L02_001B368C + (*(int *)(p + 0xC4) << 7) + 0x40));
            if (v[0] > 0.0f) {
                *(int *)(p + 0xDC) = 1;
                func_L02_002DCEA0(1, 10.0f);
            }
        }
    } else {
        if (D_L02_001B2FE0 >= 1.0f) {
            func_L00_002B9D60((int)D_00189EA0,
                              (int)(D_L02_001B368C + (*(int *)(p + 0xC4) << 7) + 0x70), 0, 1);
            *(short *)(D_00189EA0 + 0x160) = 10;
        }
    }
}

extern unsigned short *D_L02_001FC060[];

extern unsigned char *D_L02_001B359C;

/* Answers 1 when the object's group holds an armed 0x9CA entity (class byte 6, state word
   +0x2F6 == 1) that is not being freed (state byte 0xFE or 0xFD). */
int func_L02_003D18D8(unsigned char *a0) {
    unsigned short *q;
    unsigned short *p;
    unsigned char *e;

    if (a0[0x21] == 0xFF)
        return 0;
    q = D_L02_001FC060[a0[0x21]];
    if (q == 0)
        return 0;
    p = q;
    do {
        e = D_L02_001B359C + ((*p & 0x7FFF) << 8);
        if (e != 0 && e[0x20] != 0xFE && e[0x20] != 0xFD && *(short *)(e + 0xAA) == 0x9CA &&
            e[0x95] == 6 && *(short *)(*(unsigned char **)(e + 0x68) + 0x2F6) == 1)
            return 1;
    } while (*(short *)p++ >= 0);
    return 0;
}
