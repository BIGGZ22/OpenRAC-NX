extern int func_00282978(short *);
extern void func_00282CC0(char *, char *, char *);
extern void func_002B9180(void *);
extern unsigned char *D_001F26D0[];
extern float D_001AA384 __attribute__((sda));

void func_002C8268(char *self)
{
    char *state = self + 32;
    char *pos;

    if (func_00282978((short *)(self + 10)))
        *(float *)(state + 8) -= *(float *)(state + 24);
    pos = self + 16;
    func_00282CC0(pos, pos, state);
    func_00282CC0(pos, pos, *(char **)(state + 28));
    self[8] = (unsigned char)self[8] + *(unsigned char *)(state + 20);
    if ((unsigned char)self[2] == D_001F26D0[0][0])
        *(float *)(self + 12) += D_001AA384 * 3500.000244140625f;
    if (*(float *)(self + 24) < *(float *)(state + 16))
        func_002B9180(self);
}

extern void func_00282D30(char *, char *, float);
extern float func_00283CE0(int);
extern int func_00283CF0(float);
extern float D_001AA3D8 __attribute__((sda));

void func_002C7888(char *self)
{
    char *state = self + 32;
    float f;
    int colour;

    func_00282D30(state, state, D_001AA3D8);
    *(float *)(state + 8) -= *(float *)(state + 0x1C);
    func_00282CC0(self + 16, self + 16, state);
    if (*(short *)(self + 10) < *(int *)(state + 16)) {
        f = func_00283CE0(*(short *)(self + 10));
        f = f / func_00283CE0(*(int *)(state + 16));
        colour = (func_00283CF0(f * 64.0f) << 24) | 0x808080;
        *(int *)(self + 4) = colour | 0x8080;
    } else {
        colour = 0x40800000;
        *(int *)(self + 4) = colour | 0x8080;
    }
    self[8] = (unsigned char)self[8] + *(unsigned char *)(state + 20);
    if (func_00282978((short *)(self + 10)))
        func_002B9180(self);
}

#include "include_asm.h"

/* The three handler slots and the string table are read through $gp, so this
 * function needs the -G8 profile of the sibling file. */
extern int D_001AA420 __attribute__((sda));
extern int D_001AA428 __attribute__((sda));
extern int D_001AA42C __attribute__((sda));
extern char D_0025BA90[];
extern int func_002D5B38(int, char *);

#include "include_asm.h"

/*
 * The game's global state: the mode 0 case copies one of its fields into another.
 * The struct's own address is the one the retail code forms (lui/addiu) and the two
 * fields are the offsets it uses from it.
 */
typedef struct {
    char reserved_000[0x1248];
    int value_1248;
    char reserved_124C[0x107C];
    int value_22C8;
} GameState_189E20;

extern GameState_189E20 D_00189E20;

/* The menu entries the seven cases hand to func_002D5E70, in the small data area. */
extern int D_001AA6B8 __attribute__((sda));
extern int D_001AA6D0 __attribute__((sda));
extern int D_001AA6E8 __attribute__((sda));
extern int D_001AA700 __attribute__((sda));
extern int D_001AA718 __attribute__((sda));
extern int D_001AA730 __attribute__((sda));
extern int D_001AA748 __attribute__((sda));

extern int func_002D5E70(char *object, int *result);
