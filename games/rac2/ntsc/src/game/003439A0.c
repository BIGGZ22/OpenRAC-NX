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

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern void func_003361E0(void *, void *, void *);
extern void func_003369A8(void *, void *, void *);
extern int *func_00335E20(void *);
extern void func_00336A30(void *, int);
extern void func_00335F88(void *, float, float, float, float);
extern void func_00336390(void *, void *, int);
extern void func_00336710(void *, void *, void *);
extern int func_00288FB8(int);
extern void func_00336A38(void *, int);
extern int func_00344478();
extern char D_001ADC10[], D_001ADC18[], D_001ADC20[], D_001ADC28[], D_001ADFC0[], D_001ADFC8[];
extern char D_001ADF10[], D_001ADF18[], D_001ADF20[], D_001ADD60[], D_001AE250[];
extern char D_001ADD58[], D_001AE030[], D_001ADFD8[];
extern char *D_001A8CFC[] __attribute__((sda));

void func_00343A30(char *self, void *arg)
{
    int *buf;
    float zero;

    *(void **)(self + 0x434) = arg;
    if (arg) {
        buf = func_00336DA8(16, func_00336E40(arg));
        *(int **)(self + 0x430) = buf;
        buf[1] = 0;
        buf[2] = 0;
        buf[3] = 0;
        buf[0] = 0;
    }
    zero = 0.0f;
    *(int *)(self + 0x438) = 1;
    (*(float **)(self + 0x430))[0] = 250.0f;
    (*(float **)(self + 0x430))[1] = 207.0f;
    func_003361E0(self, D_001ADC10, arg);
    func_003361E0(self + 0x4C, D_001ADC18, arg);
    func_003361E0(self + 0x98, D_001ADC20, arg);
    func_003361E0(self + 0xE4, D_001ADC28, arg);
    func_003361E0(self + 0x130, D_001ADFC0, arg);
    func_003361E0(self + 0x17C, D_001ADFC8, arg);
    func_003369A8(self + 0x328, D_001ADF10, arg);
    func_003369A8(self + 0x370, D_001ADF18, arg);
    func_003369A8(self + 0x3B8, D_001ADF20, arg);
    func_003369A8(self + 0x298, D_001ADD60, arg);
    func_003369A8(self + 0x2E0, D_001AE250, arg);
    func_003369A8(self + 0x1C8, D_001ADD58, arg);
    func_003369A8(self + 0x210, D_001AE030, arg);
    *func_00335E20(self) = 0x60442D00;
    *func_00335E20(self + 0x4C) = 0x60241700;
    *func_00335E20(self + 0x98) = 0x55F0C070;
    *func_00335E20(self + 0xE4) = 0x55F0C070;
    *func_00335E20(self + 0x17C) = 0x70FFFEED;
    *func_00335E20(self + 0x130) = 0x55F0C070;
    *func_00335E20(self + 0x298) = 0x80F0F0F0;
    *func_00335E20(self + 0x2E0) = 0x80F0F0F0;
    *func_00335E20(self + 0x1C8) = 0x80F0F0F0;
    *func_00335E20(self + 0x210) = 0x80F0F0F0;
    *func_00335E20(self + 0x328) = 0x80F0F0F0;
    *func_00335E20(self + 0x370) = 0x80F0F0F0;
    *func_00335E20(self + 0x3B8) = 0x80F0F0F0;
    func_00336A30(self + 0x298, 1);
    func_00336A30(self + 0x2E0, 1);
    func_00335F88(self + 0x2E0, 0.9f, 0.9f, zero, zero);
    func_00336390(self, D_001A8CFC[0] + 0x8710, 0x9E);
    func_00336390(self + 0x4C, D_001A8CFC[0] + 0x8710, 0x9F);
    func_00336390(self + 0x98, D_001A8CFC[0] + 0x8710, 0xA0);
    func_00336390(self + 0xE4, D_001A8CFC[0] + 0x8710, 0xA1);
    func_00336390(self + 0x130, D_001A8CFC[0] + 0x8710, 0x5D);
    func_00336390(self + 0x17C, D_001A8CFC[0] + 0x8710, 0x5E);
    *(int *)(self + 0x43C) = 0;
    *(int *)(self + 0x440) = 0;
    *(int *)(self + 0x448) = 3;
    func_00336710(self + 0x258, D_001ADFD8, arg);
    *func_00335E20(self + 0x258) = 0x60F0F0B0;
    func_00335F88(self + 0x258, 32.0f, 32.0f, zero, zero);
    *func_00335E20(self + 0x1C8) = 0x80F0F0F0;
    *func_00335E20(self + 0x210) = 0x80F0F0F0;
    func_00336A38(self + 0x1C8, func_00288FB8(0x2D54));
    func_00336A38(self + 0x210, func_00288FB8(0x2D59));
    func_00336A38(self + 0x328, (int)(self + 0x400));
    func_00336A38(self + 0x370, (int)(self + 0x410));
    func_00336A38(self + 0x3B8, (int)(self + 0x420));
    func_00336A30(self + 0x328, 0);
    func_00336A30(self + 0x370, 0);
    func_00336A30(self + 0x3B8, 0);
    *(int *)(self + 0x444) = 0;
    func_00336A38(self + 0x298, func_00288FB8(0x2D4F));
    func_00344478(self, 0);
}

typedef struct { unsigned char pad[6]; short id; short tail; } SelectionRow;
extern SelectionRow D_001AA810[];

void func_00343EE8(DobboGridState312 *state, unsigned int buttons)
{
    if (buttons & 0x1000) {
        func_002E5E58(3, 0, 0);
        state->row = 1;
        state->mode = 2;
    } else if (buttons & 0x4000) {
        func_002E5E58(3, 0, 0);
        state->row = 0;
        state->mode = 2;
    } else if (buttons & 0x8000) {
        --state->column;
        func_002E5E58(3, 0, 0);
        if (state->column < 0) {
            state->column = 0;
            state->mode = 1;
        }
    } else if (buttons & 0x2000) {
        func_002E5E58(3, 0, 0);
        ++state->column;
        if (state->column >= 2) {
            state->column = 0;
            state->mode = 1;
            state->row = 0;
        }
    }
    state->index = D_001AA810[state->column].id;
}

/* The cursor's cell table: the same ten-byte record shape as the selection rows above, but the
   table's address is the level's own, so it is a ref_ name. */
extern SelectionRow ref_L00_00286D68_00[];

/* The cursor's up/down/left/right move: the button mask picks the direction, the state keeps
   the column (0x43C), the row (0x440), the chosen index (0x444) and the mode (0x448). */
void func_00343FF8(DobboGridState312 *state, unsigned int buttons)
{
    if (buttons & 0x1000) {
        func_002E5E58(3, 0, 0);
        if (--state->row < 0) {
            if (state->column < 2) {
                state->row = 0;
                state->mode = 0;
            } else if (state->column == 2) {
                state->row = 1;
            } else {
                state->row = 0;
                state->mode = 1;
                state->column = 0;
            }
        }
    } else if (buttons & 0x4000) {
        func_002E5E58(3, 0, 0);
        if (++state->row >= 2) {
            if (state->column < 2) {
                state->row = 0;
                state->mode = 0;
            } else if (state->column == 2) {
                state->row = 0;
            } else {
                state->row = 0;
                state->mode = 3;
                state->column = 0;
            }
        }
    } else if (buttons & 0x8000) {
        func_002E5E58(3, 0, 0);
        if (--state->column < 0) {
            state->column = 3;
        }
    } else if (buttons & 0x2000) {
        func_002E5E58(3, 0, 0);
        if (++state->column >= 4) {
            state->column = 0;
            state->mode = 3;
            state->row = 0;
        }
    }
    state->index = ref_L00_00286D68_00[state->column + state->row * 4].id;
}

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

/* Per-mode lookup of the selected entry's id (the short at +0x42 of a 0xE0-byte record).
   Retail reads the mode 1 flag byte through a bare small-data symbol, so it is declared sda
   under its own name (D_001A7AFE, the byte at +6 of the D_001A7AF8 flags) in this -G0 file. */
extern unsigned char D_00139568[];
extern unsigned char D_001A7AF8[];
extern unsigned char D_001A7AFE __attribute__((sda));
extern unsigned char D_00239BA0[];
typedef struct { unsigned char pad[0x42]; short field42; unsigned char rest[0xE0 - 0x44]; } Entry224;
/* The four text ids, copied from their retail data: the build keeps no compiler .rodata. */
typedef struct { int v[4]; } Ids_00344378;
extern Ids_00344378 D_001AE260;

int func_00344378(DobboGridState312 *state)
{
    Ids_00344378 table = D_001AE260;
    int value;

    switch (state->mode) {
    case 0:
    case 2: {
        int index = state->index;
        if (D_001A7AF8[index] != 0) {
            value = ((Entry224 *)D_00239BA0)[D_00139568[index]].field42;
            break;
        }
        return 0;
    }
    case 1:
        if (D_001A7AFE != 0) {
            value = ((Entry224 *)D_00239BA0)[D_00139568[6]].field42;
            break;
        }
        return 0;
    case 3:
        value = table.v[state->index];
        break;
    default:
        value = 0;
        break;
    }
    return value;
}

