#include "include_asm.h"

extern void func_0033AF48_2();

extern void func_002DAF68_1();

extern void func_00336A78_5();

extern void func_00336828();

extern void func_00336320_4();

extern void func_0033ED88();

typedef unsigned char u8;

extern void func_L00_0042BAD0_6(u8 *);

extern void func_L00_0042BFF8_5(u8 *);

extern void func_L00_0042C290_6(u8 *);

extern void func_00347EA0_1(u8 *);

typedef int s32;

/* Forwards one object pointer, eight pointer-plus-constant arguments and a null to ten calls in a fixed order. */
void func_0033F030(char *p)
{
    func_0033AF48_2(p + 8);
    func_002DAF68_1(0);
    func_00336A78_5(p + 744);
    func_00336A78_5(p + 816);
    func_00336A78_5(p + 888);
    func_00336828(p + 684);
    func_00336320_4(p + 960);
    func_00336A78_5(p + 1400);
    func_00336A78_5(p + 1328);
    func_0033ED88(p);
}

extern void *func_003361A8(void *);
extern char *func_00336968(char *);
extern void *func_003363D0(void *);
extern void *func_003366D0(void *);
char *func_0033F0A0(char *object)
{
    char *cursor;
    int i;
    char *a, *b, *c, *d, *e, *g, *f;
    func_003361A8(object);
    func_003361A8(object + 0x4C);
    func_003361A8(object + 0x98);
    func_003361A8(object + 0xE4);
    func_003361A8(object + 0x130);
    func_003361A8(object + 0x17C);
    func_003361A8(object + 0x1C8);
    func_003361A8(object + 0x214);
    func_003363D0(object + 0x260);
    func_003361A8(object + 0x2A8);
    func_003361A8(object + 0x2F4);
    cursor = object + 0x340;
    for (i = 7; i != -1; i--, cursor += 0x4C) func_003361A8(cursor);
    func_003361A8(object + 0x5A0);
    cursor = object + 0x5EC;
    i = 7;
    f = object + 0x948;
    a = object + 0x7D0;
    b = object + 0x828;
    g = object + 0x990;
    c = object + 0x870;
    d = object + 0x8B8;
    e = object + 0x900;
    for (; i != -1; i--, cursor += 0x3C) func_003366D0(cursor);
    func_003366D0(a);
    func_00336968(b);
    func_00336968(c);
    func_00336968(d);
    func_00336968(e);
    func_00336968(f);
    func_00336968(g);
    return object;
}


INCLUDE_ASM("asm/game", func_0033F210);

INCLUDE_ASM("asm/game", func_0033F218);

INCLUDE_ASM("asm/game", func_0033FB98);

INCLUDE_ASM("asm/game", func_00340250);

INCLUDE_ASM("asm/game", func_00340498);

INCLUDE_ASM("asm/game", func_00340638);

INCLUDE_ASM("asm/game", func_003407F8);


/* a menu's table entry: ten bytes, the value a signed short at offset six */
typedef struct {
    char reserved00[6];
    short value;
    char reserved08[2];
} MenuEntry;

/* a menu's state: the selected entry's column and page, a flag and the entry's value */
typedef struct {
    char reserved00[0x814];
    int column;
    int page;
    int flag;
    int value;
} MenuState;

/* the table, in the memory a program replaces from 0x1A7C80 on: every program has
   its own copy of it at that address */
extern MenuEntry D_001FCE0[];
extern int func_002E5E58(int, unsigned int, void *);
extern void func_003407F8(MenuState *, MenuEntry *, int);

/* Moves the selection of a menu's table: the buttons 0x1000 and 0x4000 move the page,
   0x8000 and 0x2000 the column and 0x40 asks for the entry's value, which is then kept
   in the state. The page moves compare against a variable, so that the compiler keeps
   the -1 in a register, as the retail code does. */
void func_00340B70(MenuState *state, unsigned int buttons)
{
    if (buttons & 0x1000) {
        int v = state->page - 1;
        int lower = -1;
        v = (v > lower) ? v : 1;
        state->page = state->column ? v : 0;
        func_002E5E58(3, 0, 0);
    } else if (buttons & 0x4000) {
        int v = state->page + 1;
        v = (v > 1) ? 0 : v;
        state->page = state->column ? v : 0;
        func_002E5E58(3, 0, 0);
    } else if (buttons & 0x8000) {
        state->column = state->column - 1;
        func_002E5E58(3, 0, 0);
        if (state->column < 0 || (state->column == 0 && state->page == 1)) {
            state->column = 3;
            state->page += 4;
            state->flag = 0;
        }
    } else if (buttons & 0x2000) {
        state->column = state->column + 1;
        func_002E5E58(3, 0, 0);
        if (state->column >= 3) {
            state->column = 0;
            state->page += 4;
            state->flag = 0;
        }
    } else if (buttons & 0x40) {
        func_003407F8(state, D_001FCE0, 3);
    }
    state->value = D_001FCE0[state->column + 3 * state->page].value;
}

extern void *func_003361A8(void *);
extern char *func_00336968(char *);
extern void *func_003363D0(void *);
extern void *func_003366D0(void *);
u8 *func_00340D18(u8 *self)
{
    func_003361A8(self);
    func_003361A8(self + 76);
    func_00336968((char *)self + 152);
    func_003361A8(self + 224);
    func_003363D0(self + 300);
    func_003366D0(self + 372);
    return self;
}

INCLUDE_ASM("asm/game", func_00340D68);

INCLUDE_ASM("asm/game", func_00341030);

INCLUDE_ASM("asm/game", func_003411B0);

/* initialises eight in-place sub-records of one object by four routines, then returns the object pointer */
u8 *func_003411C0(u8 *a0) {
    func_L00_0042BAD0_6(a0);
    func_L00_0042BAD0_6(a0 + 76);
    func_L00_0042BAD0_6(a0 + 152);
    func_L00_0042BAD0_6(a0 + 228);
    func_L00_0042BAD0_6(a0 + 304);
    func_L00_0042BFF8_5(a0 + 380);
    func_L00_0042C290_6(a0 + 464);
    func_00347EA0_1(a0 + 552);
    return a0;
}


/* stores three consecutive words; byte-identical across all 27 levels. */
void func_00341540(u8 *a0, s32 a1, s32 a2, s32 a3) { *(s32 *)(a0 + 0x2F0) = a1; *(s32 *)(a0 + 0x2F4) = a2; *(s32 *)(a0 + 0x2F8) = a3; }

/* Measured target: byte-identical across all 27 levels. */
void func_00341550(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x220) = a1; }



INCLUDE_ASM("asm/game", func_00341610);



void func_003417C8(void)
{
}
