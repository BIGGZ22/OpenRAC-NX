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

extern char D_L00_0021DE10[];

void func_L00_00358E50(void)
{
    func_L00_00358D98(D_L00_0021DE10);
}

INCLUDE_ASM("asm/levels", func_L00_0035CC98);

INCLUDE_ASM("asm/levels", func_L00_0035D168);

extern int D_001B19E8 __attribute__((sda));

extern int D_001B19EC __attribute__((sda));

extern int D_001B8F0C __attribute__((sda));

extern char D_00188660_3[];

extern int func_002E5AD0(char *a0, int a1, int a2, int a3, int a4);

/* Shared slot table: the first free record of a 112-byte table is filled with the index and the
 * caller's value; returns the record number, or -1 when the index is out of range. */
int func_L00_0035F5A0(int index, int a1, int a2)
{
    int r;
    char *p;

    if (D_001B19EC == 0)
        return -1;
    index = index + D_001B8F0C;
    if (index >= D_001B19E8)
        return -1;
    r = func_002E5AD0((char *)(D_001B19EC + index * 32), a1, a2, 0, 1024);
    if (r >= 0) {
        p = D_00188660_3 + r * 112;
        *(int *)(p + 136) = a2;
        *(short *)(p + 126) = index;
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L00_0035F640);

/* Builds a slot index out of three weight/kind arguments and fills the same 112-byte record as
 * above; the record's value word comes from the fifth argument, the helper's from the sixth. */
int func_L00_0035F6D8(int a0, int a1, int a2, int a3, int a4, int a5)
{
    int index;
    int sum;
    int r;
    char *p;

    if (a0 == 0xFFFFFFFFU)
        return -1;
    sum = a0 * 4 + a1 * 2;
    sum += a2;
    index = sum + D_001B8F0C;
    if (index >= D_001B19E8)
        return -1;
    r = func_002E5AD0((char *)(D_001B19EC + index * 32), a3, a4, 0, a5);
    if (r >= 0) {
        p = D_00188660_3 + r * 112;
        *(int *)(p + 136) = a4;
        *(short *)(p + 126) = index;
    }
    return r;
}

INCLUDE_ASM("asm/levels", func_L00_0035F7E8);

void func_L00_0035F8D0(int value60, int selector68, int value64, int axis69, int mode6A)
{
    Rac2Native_acdcf1600d770d3b_ScalarConfigure116State *state = D_00188660_2;
    state->value_60 = value60;
    if (state->selector_68 != selector68) {
        state->selector_68 = selector68;
        state->dirty_6b |= 1;
    }
    if (state->value_64 != value64) {
        state->value_64 = value64;
        state->dirty_6b |= 4;
    }
    if (state->axis_69 != axis69 || state->mode_6a != mode6A) {
        state->mode_6a = mode6A;
        state->axis_69 = axis69;
        state->dirty_6b |= 2;
    }
}

void func_L00_0035F9F0(int value, long address) {
    int *target = (int *)(int)address;
    if (target != 0)
        *target = value;
}

INCLUDE_ASM("asm/levels", func_L00_0035FA80);

INCLUDE_ASM("asm/levels", func_L00_003603C0);

INCLUDE_ASM("asm/levels", func_L00_00364EE0);

extern void func_002B7B18(int value, char *target);

extern void func_002B7CE0(int value);

extern int D_L00_001BCAC0[];

extern int D_0014B540[];

/* Same body as func_L00_003046D0: compares two words of a 332-byte-stride table and calls the
 * two helpers when the entry ahead is greater; returns 1. */
int func_L00_00364EE8(int index)
{
    int j = index + 1;
    int *d = D_L00_001BCAC0;
    int *b = D_0014B540;
    int *t = (int *)((char *)b + 16040);
    int n = d[12] * 332;
    int hold = d[28];
    int j4 = j * 4;
    int cur = *(int *)((char *)t + (n + index * 4));
    int next = *(int *)((char *)t + (j4 + n));

    n = next - cur;
    if (n > 0) {
        func_002B7B18(hold, (char *)(cur + b[3977]));
        func_002B7CE0(0);
    }
    return 1;
}

extern unsigned short D_00152C26;

void func_L00_003672F8(void) {
    D_00152C26 = 1;
}

/* func_L00_00367308 is defined in L00_00358D98-g8.c. */

#define NOT_SDA __attribute__((section(".data")))

extern void *r1_D_0012F86C_00367720 NOT_SDA;

/* rac1-decomp src/core/001138A8.c: func_001138A8 */
int func_L00_00367720(void) {
    return (int)r1_D_0012F86C_00367720;
}

INCLUDE_ASM("asm/levels", func_L00_00368F58);



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

extern unsigned int D_001A7424 __attribute__((sda));

/* The level's own resident word and resident object: each place resolves its own value from
 * the retail bytes (0x001A8F00 and 0x001AC0C0 in 00_aranos_tutorial), so both are sda. */
extern int ref_L00_001A8F00_00 __attribute__((sda));

typedef struct L00_0036FA68_Object {
    unsigned char pad00[0x64];
    void (*field64)(struct L00_0036FA68_Object *);
} L00_0036FA68_Object;

extern L00_0036FA68_Object *ref_L00_001AC0C0_00 __attribute__((sda));

extern void func_00284E18(int a, int b, int c);

extern int func_002B49E0(int a, int b, int c, int d, int e);

extern void func_002F5220(void);

extern void func_002E4AC8(void);

extern void func_002B9390(void);

extern void func_L00_00310048(void);

/* The tick gate's callee is a neighbouring local function in six of the programs. */
extern unsigned int ref_L00_0036FA68_00(void);

extern void func_002F52B8(void);

extern void func_00289178(void);

/* Runs the level's tick chain once: the three-argument engine reset, a gate on the resident
 * word and the boot flag, then the subsystem updates and the resident object's callback. */
void func_L00_0036FA68(void)
{
    L00_0036FA68_Object *object;

    func_00284E18(0, 0, 0);
    if (ref_L00_001A8F00_00 == 0 && (D_001A7424 & 1)) {
        if (func_002B49E0(4, 1, 1, 0, 0) == 0)
            return;
    }
    if (ref_L00_0036FA68_00() != 0)
        return;
    func_002F5220();
    func_002E4AC8();
    func_002B9390();
    func_L00_00310048();
    object = ref_L00_001AC0C0_00;
    object->field64(object);
    func_002F52B8();
    func_00289178();
}

INCLUDE_ASM("asm/levels", func_L00_00370408);

INCLUDE_ASM("asm/levels", func_L00_00373D40);

void func_L00_00373F40(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00373F48);

INCLUDE_ASM("asm/levels", func_L00_00374348);

extern int D_0022F810;

/* Dispatch on a mode word: mode 1 goes through the recording wrapper, the other modes replay. */
void func_L00_00374B98(void)
{
    switch (D_0022F810) {
    case 0:
        func_002F93B0();
        break;
    case 1:
        func_L00_00374B78();
        break;
    case 2:
        func_002F93B0();
        break;
    case 3:
        func_002F93B0();
        break;
    }
}

extern void func_3747B0(void);

void func_L00_00374B78(void) {
    func_3747B0();
}

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
