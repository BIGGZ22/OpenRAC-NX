#include "include_asm.h"

extern void F449e2f67_FUN_003361A8(char *p);

extern void F449e2f67_FUN_00336968(char *p);

extern void F449e2f67_FUN_003366D0(char *p);

/* Update the observed two-axis selection fields and their combined index. */
typedef struct {
    unsigned char gap0[0x43c];
    int column, row, index, mode;
} DobboGridState312;

extern int func_002E5E58(int, unsigned int, void *);

char *func_003439A0(char *p)
{
    F449e2f67_FUN_003361A8(p);
    F449e2f67_FUN_003361A8(p + 76);
    F449e2f67_FUN_003361A8(p + 152);
    F449e2f67_FUN_003361A8(p + 228);
    F449e2f67_FUN_003361A8(p + 304);
    F449e2f67_FUN_003361A8(p + 380);
    F449e2f67_FUN_00336968(p + 456);
    F449e2f67_FUN_00336968(p + 528);
    F449e2f67_FUN_003366D0(p + 600);
    F449e2f67_FUN_00336968(p + 664);
    F449e2f67_FUN_00336968(p + 736);
    F449e2f67_FUN_00336968(p + 808);
    F449e2f67_FUN_00336968(p + 880);
    F449e2f67_FUN_00336968(p + 952);
    return p;
}

INCLUDE_ASM("asm/game", func_00343A30);

INCLUDE_ASM("asm/game", func_00343EE8);

INCLUDE_ASM("asm/game", func_00343FF8);

/* prioritized masks1000/4000/8000/2000 update signedcolumns43C/rows440 and mode448 with originalevent3 flags0/entitynull; index444=column+2row */
void func_00344160(DobboGridState312 *state, unsigned int buttons)
{
    if (buttons & 0x1000) {
        func_002E5E58(3, 0, 0);
        --state->row;
        if (state->row < 0) {
            if (state->column == 0) {
                state->mode = 2;
                state->row = 1;
                state->column = 3;
            } else if (state->column == 1) {
                state->row = state->column;
            }
        }
    } else if (buttons & 0x4000) {
        int row;
        func_002E5E58(3, 0, 0);
        row = state->row + 1;
        state->row = row > 1 ? 0 : row;
    } else if (buttons & 0x8000) {
        func_002E5E58(3, 0, 0);
        --state->column;
        if (state->column < 0) {
            state->column = 3;
            state->row = 1;
            state->mode = 2;
        }
    } else if (buttons & 0x2000) {
        func_002E5E58(3, 0, 0);
        ++state->column;
        if (state->column >= 2)
            state->column = 0;
    }
    state->index = state->column + state->row * 2;
}

extern short D_L00_001AAB76;
void func_00344298(char *object, unsigned int buttons)
{
    if (buttons & 0x1000) {
        func_002E5E58(3, 0, 0);
        *(int *)(object + 0x448) = 2;
        *(int *)(object + 0x440) = 1;
        *(int *)(object + 0x43C) = 3;
    } else if (buttons & 0x4000) {
        func_002E5E58(3, 0, 0);
        *(int *)(object + 0x448) = 2;
        *(int *)(object + 0x43C) = 3;
        *(int *)(object + 0x440) = 0;
    } else if (buttons & 0x8000) {
        func_002E5E58(3, 0, 0);
        *(int *)(object + 0x448) = 0;
        *(int *)(object + 0x43C) = 1;
        *(int *)(object + 0x440) = 0;
    } else if (buttons & 0x2000) {
        func_002E5E58(3, 0, 0);
        *(int *)(object + 0x440) = 0;
        *(int *)(object + 0x448) = 3;
        *(int *)(object + 0x43C) = 0;
    }
    *(int *)(object + 0x444) = D_L00_001AAB76;
}

INCLUDE_ASM("asm/game", func_00344378);

INCLUDE_ASM("asm/game", func_00344478);
