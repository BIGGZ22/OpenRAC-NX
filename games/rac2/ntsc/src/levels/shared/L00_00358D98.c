#include "include_asm.h"

extern char *func_L00_00358D50(int a0, int a1);

extern void func_00282AB0_1(char *target, char *base, int len);

typedef struct {
    short key;
    short index;
    int value;
} Query;

typedef struct {
    int key;
    int value;
} Entry32fd6f60;

extern Entry32fd6f60 D_L07_002A4100[];

/* Measured partial layouts; original type and game roles are unknown. */
typedef struct Rac2Native_acdcf1600d770d3b_ScalarConfigure116State {
    unsigned char prefix_00[0x60];
    unsigned int value_60;
    int value_64;
    unsigned char selector_68;
    unsigned char axis_69;
    unsigned char mode_6a;
    unsigned char dirty_6b;
} Rac2Native_acdcf1600d770d3b_ScalarConfigure116State;

extern Rac2Native_acdcf1600d770d3b_ScalarConfigure116State D_00188660_2[];

/* Walks up to 25 records of 10 bytes; for each record whose 16-bit key is non-negative and whose table byte is set, resolves an object through a helper and copies/relocates it with the record's 16-bit length, capping the destination table at 25 entries. */
void func_L00_00358D98(char *a0)
{
    int i = 0;

    if (i < *(int *)a0) {
        int one = 1;
        char *d = a0 + 8;
        char *e = a0 + 4;

        do {
            char *obj = func_L00_00358D50(*(short *)(e + 14), *(short *)(e + 12));
            if (obj != 0) {
                char *t;
                if (*(short *)(e + 10) == one)
                    t = (char *)(*(int *)e + (int)obj);
                else
                    t = (char *)(*(int *)e + *(int *)(obj + 104));
                func_00282AB0_1(t, d, *(unsigned short *)(e + 8));
            }
            i++;
            d += 16;
            e += 16;
        } while (i < *(int *)a0);
    }
}

INCLUDE_ASM("asm/levels", func_L00_00358E50);

INCLUDE_ASM("asm/levels", func_L00_0035CC98);

INCLUDE_ASM("asm/levels", func_L00_0035D168);

INCLUDE_ASM("asm/levels", func_L00_0035F5A0);

INCLUDE_ASM("asm/levels", func_L00_0035F640);

INCLUDE_ASM("asm/levels", func_L00_0035F6D8);

/* shared native body of family 32fd6f604b71fbbf */
void func_L00_0035F7E8(Query *query)
{
    int i;

    i = 0;
    while (D_L07_002A4100[i].key != -1 && D_L07_002A4100[i].key != query->key)
        i++;
    query->index = i;
    query->value = D_L07_002A4100[i].value;
}

/* Update fields at offsets 0x60 through 0x6A and set the corresponding dirty bits when selector, value, axis or mode changes; original game role is unknown. */
void func_L00_0035F8D0(unsigned int value, unsigned int selector, int other,
                        unsigned int axis, unsigned int mode)
{
    Rac2Native_acdcf1600d770d3b_ScalarConfigure116State *state = D_00188660_2;
    state->value_60 = value;
    if (state->selector_68 != selector) {
        state->dirty_6b |= 1;
        state->selector_68 = selector;
    }
    if (state->value_64 != other) {
        state->dirty_6b |= 4;
        state->value_64 = other;
    }
    if (state->axis_69 != axis || state->mode_6a != mode) {
        state->dirty_6b |= 2;
        state->axis_69 = axis;
        state->mode_6a = mode;
    }
}

INCLUDE_ASM("asm/levels", func_L00_0035F9F0);

INCLUDE_ASM("asm/levels", func_L00_0035FA80);

INCLUDE_ASM("asm/levels", func_L00_003603C0);

INCLUDE_ASM("asm/levels", func_L00_00364EE0);

INCLUDE_ASM("asm/levels", func_L00_00364EE8);

INCLUDE_ASM("asm/levels", func_L00_003672F8);

INCLUDE_ASM("asm/levels", func_L00_00367308);

INCLUDE_ASM("asm/levels", func_L00_00367720);

INCLUDE_ASM("asm/levels", func_L00_00368F58);

INCLUDE_ASM("asm/levels", func_L00_0036ADB0);

INCLUDE_ASM("asm/levels", func_L00_0036AE00);

INCLUDE_ASM("asm/levels", func_L00_0036BE28);

INCLUDE_ASM("asm/levels", func_L00_0036C9AC);

INCLUDE_ASM("asm/levels", func_L00_0036C9D4);

INCLUDE_ASM("asm/levels", func_L00_0036D274);

INCLUDE_ASM("asm/levels", func_L00_0036D29C);

INCLUDE_ASM("asm/levels", func_L00_0036EF40);

INCLUDE_ASM("asm/levels", func_L00_0036EF68);

unsigned int func_L00_0036F480(void)
{
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_0036F488);

INCLUDE_ASM("asm/levels", func_L00_0036FA68);

INCLUDE_ASM("asm/levels", func_L00_00370408);

INCLUDE_ASM("asm/levels", func_L00_00373D40);

void func_L00_00373F40(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00373F48);

INCLUDE_ASM("asm/levels", func_L00_00374348);

INCLUDE_ASM("asm/levels", func_L00_00374708);

INCLUDE_ASM("asm/levels", func_L00_00374B78);

INCLUDE_ASM("asm/levels", func_L00_00374B98);

INCLUDE_ASM("asm/levels", func_L00_00374C18);

INCLUDE_ASM("asm/levels", func_L00_00375718);

INCLUDE_ASM("asm/levels", func_L00_00376600);

INCLUDE_ASM("asm/levels", func_L00_003769C0);

INCLUDE_ASM("asm/levels", func_L00_00377330);

INCLUDE_ASM("asm/levels", func_L00_00377580);

INCLUDE_ASM("asm/levels", func_L00_003778F0);

INCLUDE_ASM("asm/levels", func_L00_00377C50);

INCLUDE_ASM("asm/levels", func_L00_003781B0);

INCLUDE_ASM("asm/levels", func_L00_00378388);

INCLUDE_ASM("asm/levels", func_L00_00378578);

INCLUDE_ASM("asm/levels", func_L00_00378818);

INCLUDE_ASM("asm/levels", func_L00_00378C18);

INCLUDE_ASM("asm/levels", func_L00_00378DE0);

INCLUDE_ASM("asm/levels", func_L00_00378E90);

INCLUDE_ASM("asm/levels", func_L00_0037A2E8);

INCLUDE_ASM("asm/levels", func_L00_0037A610);

void func_L00_0037AF58(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_0037AFB0);

INCLUDE_ASM("asm/levels", func_L00_0037B408);

INCLUDE_ASM("asm/levels", func_L00_0037B4B8);

INCLUDE_ASM("asm/levels", func_L00_0037B5C0);

INCLUDE_ASM("asm/levels", func_L00_0037B6E8);
