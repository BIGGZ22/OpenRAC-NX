#include "include_asm.h"

/* Qwen draft (branch qwen/autonomy, 385de342), checked by tools/check.py */
void func_002D5B58(short param_1, int param_2, int *param_3);

/* func_002D7AC8: owner ABI/declaration review retained. */
extern int D_001C51D4;

extern void F70997ba9_FUN_002FC560(int a, int b);

extern void F70997ba9_FUN_0027C540(int a);

extern void *F70997ba9_FUN_00288FB8(int id);

extern int F70997ba9_FUN_0027F198(void *p, int i);

extern void F70997ba9_FUN_0027F120(void);

extern void F70997ba9_FUN_0027F5B8(int a, int b, long c, void *d, int e);

extern void F70997ba9_FUN_0027F110(void);

extern void F70997ba9_FUN_0027C660(void);

typedef int s32;

extern s32 func_002DE550 (s32);

/* Qwen draft (branch qwen/autonomy, f0b23b24), checked by tools/check.py */
typedef unsigned long long undefined8;

typedef unsigned int undefined4;

extern undefined4 func_002DE618(undefined4);

#ifndef RAC2_T_TWO_F73AF1964
#define RAC2_T_TWO_F73AF1964
typedef struct { signed char a, b; } TWO_F73af1964;
#endif

extern void func_0027C540_1(int);

extern void func_0027F538(int, int, long, signed char *, int);

extern int  func_0028E588(int, int);

extern int  func_0028E640(int);

extern void func_0028F410(float, float, float, float, float, int, int, int);

extern void func_0027C660_1(void);

extern signed char D_L01_001ABE90[];

extern signed char D_L01_001ABE98[];

extern int func_002DE3A0(int);

extern int func_002DE550_1(int);

extern int func_002C9A60(void);

extern int func_L00_002F3478(void);

extern int func_00339760(int);

extern int D_L00_001A904C_8 __attribute__((sda));

void func_002D5E98(int param_1)
{
  int auStack_20[4];

  auStack_20[0] = 0;
  func_002D5B58(*(short *)(param_1 + 2), *(int *)(param_1 + 4), &auStack_20[0]);
  return;
}

INCLUDE_ASM("asm/game", func_002D5EC0);

INCLUDE_ASM("asm/game", func_002D6230);

/* func_002D68D8 is in 002D5E98-g8.c. */

INCLUDE_ASM("asm/game", func_002D6970);

typedef struct { char pad[4]; short mode; } Mode2D6E78;
typedef struct { char pad[0x29C]; int flags[5]; int selected; } Map2D6E78;
typedef struct { char pad[0xDB]; unsigned char active; } State2D6E78;
extern Mode2D6E78 D_001A63A8;
extern Map2D6E78 D_001C4FA0;
extern State2D6E78 D_001F2840;
extern unsigned char D_001F291B;
extern void func_002B7AD0(void);

int func_002D6E78(void)
{
    if (D_001A63A8.mode == 0) {
        Map2D6E78 *map = &D_001C4FA0;
        int index = map->selected;
        if (index != -1) {
            D_001F291B = 0;
            map->flags[index] ^= 0x1000;
            map->selected = -1;
        }
    }
    if (D_001A63A8.mode != 0) {
        State2D6E78 *state = &D_001F2840;
        if (state->active != 0) {
            Map2D6E78 *map;
            func_002B7AD0();
            state->active = 0;
            map = &D_001C4FA0;
            map->flags[map->selected] = -1;
            map->selected = -1;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_002D6F38);

extern void func_0029D830(int *);
extern int func_0029DB28(unsigned int, unsigned int, unsigned int, int);
extern int D_001C51D0;
typedef struct { char pad[0x34]; int f34; int f38; int f3C; } Base2D75E8;
extern Base2D75E8 D_0025A6E0;
extern int D_0025A620[];

int func_002D75E8(int *arg0)
{
    func_0029D830(arg0);
    *(int *)((char *)arg0 + 0xA0) = func_0029DB28(0x70000000, 0, 0x70000100, 1);
    {
        char *base = (char *)arg0 + 0x30;
        int index = D_001C51D0;

        if (*(int *)(base + (index << 2)) != -1) {
            D_0025A6E0.f3C = -0x240;
            D_0025A6E0.f34 = *(int *)(0x70000000 + 4 * *(int *)(base + (index << 2)));
            *(int *)D_0025A620 = *(int *)(0x70000100 + 4 * *(int *)(base + (index << 2)));
        }
    }
    *(int *)((char *)arg0 + 0xA4) = 0;
    return 0;
}

INCLUDE_ASM("asm/game", func_002D7698);

INCLUDE_ASM("asm/game", func_002D7AC0);

int func_002D7AC8(void)
{
    D_001C51D4 = -1;
    return 0;
}

int func_002D7AE0(void) {
    func_002DE3A0(1);
    return 0;
}

extern int D_001A7BA0[] __attribute__((sda));
extern int D_001886B0[];

int func_002D7B00(void) {
    D_001886B0[0] = D_001A7BA0[0];
    return 0;
}

INCLUDE_ASM("asm/game", func_002D7B18);

INCLUDE_ASM("asm/game", func_002D7DB8);

INCLUDE_ASM("asm/game", func_002D8120);

int func_002D81A8(void) {
    return 0;
}

INCLUDE_ASM("asm/game", func_002D81B0);

extern short D_001A63AC;
extern void func_002B7AD0(void);
extern int D_001B1840 __attribute__((sda));
extern int D_00254EA4;

int func_002D82C8(char *object)
{
    if (D_001A63AC != 0 && *(int *)(object + 0x50) == 1)
        func_002B7AD0();
    func_002DE3A0(1);
    {
        int value = *(int *)(object + 0x54);
        if (value != 0) {
            D_00254EA4 = *(int *)(object + 0x38);
            D_001B1840 = value;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_002D8338);

INCLUDE_ASM("asm/game", func_002D84B0);

INCLUDE_ASM("asm/game", func_002D89D8);

extern char *D_001A8CFC[] __attribute__((sda));
extern unsigned char D_00139568[];
extern int D_00139868[];
extern unsigned char D_00239BA0[];
typedef struct { unsigned char reserved[0x6C]; int handle; } WidgetClass2D8EC0;
extern void *func_0034E5B8(void *);
extern void func_00335E68(unsigned char *, int);
extern void func_00336B98(char *, int);
extern void func_00336B90(unsigned char *, int);
extern void func_0034E220(char *, int, int, int, int);
void func_002D8EC0(char *source, short *row, int first, int second, int x, int y)
{
    int index = second * *(int *)(source + 0x44) + first;
    unsigned char *widget = (unsigned char *)func_0034E5B8(D_001A8CFC[0] + 0x36F28) + index * 0x48;
    func_00335E68(widget, 0);
    if (((WidgetClass2D8EC0 *)(D_00139568[row[3]] * 0xE0 + (unsigned int)D_00239BA0))->handle != 0) {
        func_00335E68(widget, 1);
        func_00336B98((char *)widget, D_00139868[row[3]] >> 5);
        func_00336B90(widget, ((WidgetClass2D8EC0 *)(D_00139568[row[3]] * 0xE0 + (unsigned int)D_00239BA0))->handle);
    }
    func_0034E220(D_001A8CFC[0] + 0x36F28, index, x, y, 0x80);
}


INCLUDE_ASM("asm/game", func_002D9008);

#define NOT_SDA __attribute__((section(".data")))

extern unsigned char r1_D_001414F4 NOT_SDA;

/* A short-returning inline keeps retail's branch: the select happens in
   HImode, which has no movcc pattern, so jump.c cannot turn it into the
   xori/movz a promoted `short` local gets. */
static inline short pauseFlagState_r002D95D8(void) {
    if (r1_D_001414F4 != 1) {
        return 3;
    }
    return 0;
}

/* rac1-decomp src/game/pause.c: func_0021EDD8 */
/* The select sits in a `static inline short` helper so that it stays a
   branch: jump.c turns an if into movz/movn only when its arm sets a
   full register, and the inline's short return value is a subreg. */
int func_002D95D8(char *arg0) {
    char *p = *(char **)(arg0 + 0x34);
    *(short *)(p + 2) = pauseFlagState_r002D95D8();
    return 0;
}

INCLUDE_ASM("asm/game", func_002D9600);

INCLUDE_ASM("asm/game", func_002D9730);

int func_002D9798(int *arg0)
{
    int min;
    int v;
    int slot;
    int count;
    int off;

    F70997ba9_FUN_002FC560(66, 68);
    F70997ba9_FUN_002FC560(71, 11);
    F70997ba9_FUN_0027C540(0);
    min = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11613), -1);
    v = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11625), -1);
    if (v >= min)
        min = v;
    if (*(int *)0x1A79F0 != 0) {
        v = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11614), -1);
        if (v >= min)
            min = v;
    }
    v = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11626), -1);
    if (v >= min)
        min = v;
    v = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11627), -1);
    if (v >= min)
        min = v;
    v = F70997ba9_FUN_0027F198(F70997ba9_FUN_00288FB8(11599), -1);
    if (v >= min)
        min = v;
    slot = (arg0[8] - min) >> 1;
    if (slot <= 1)
        slot = 2;
    count = arg0[9] / (*(int *)0x1A79F0 != 0 ? 7 : 6);
    F70997ba9_FUN_0027F120();
    off = count - 6;
    F70997ba9_FUN_0027F5B8(slot, off, 0x80FFA888L, F70997ba9_FUN_00288FB8(11613), -1);
    off += count;
    F70997ba9_FUN_0027F5B8(slot, off, 0x80FFA888L, F70997ba9_FUN_00288FB8(11625), -1);
    off += count;
    F70997ba9_FUN_0027F5B8(slot, off, 0x80FFA888L, F70997ba9_FUN_00288FB8(11626), -1);
    off += count;
    F70997ba9_FUN_0027F5B8(slot, off, 0x80FFA888L, F70997ba9_FUN_00288FB8(11627), -1);
    off += count;
    F70997ba9_FUN_0027F5B8(slot, off, 0x80FFA888L, F70997ba9_FUN_00288FB8(11599), -1);
    F70997ba9_FUN_0027F110();
    F70997ba9_FUN_0027C660();
    return 2;
}

INCLUDE_ASM("asm/game", func_002D99E8);



INCLUDE_ASM("asm/game", func_002D9DA0);

/* The screen position the object is drawn at, and the 64-bit packet its draw call carries. */
typedef struct {
    char pad[0x160];
    short x;
    short y;
} ScreenPos_2DA118;

extern ScreenPos_2DA118 D_001A6480;
extern unsigned long long D_001C5208;

typedef struct {
    char pad[0x34];
    int flags;
    int width;
    int height;
    char pad2[4];
    int state;
    char pad3[0x10];
    int count;
    int value;
} Object_2DA118;

extern void func_0027E018(int, char *, int, int, int, int, int, int, unsigned long long, unsigned long long);

/* Draws the object at the screen position in D_001A6480, sized by the object's own width
   and height. The draw's colour is a grey 0x808080 whose top byte comes from the object's
   counter: with the 0x40000 flag set it is the counter itself, scaled and clamped, and
   without it the colour is plain 0x80808080. Returns 0 when the object draws nothing. */
int func_002DA118(Object_2DA118 *object)
{
    int state = object->state;
    int shade;
    unsigned long long color;

    if (state < 2) {
        return 0;
    }
    if (object->count < 0) {
        return 0;
    }
    if ((object->flags & 0x40000) != 0) {
        if (state == 2) {
            shade = object->value << 5;
            if (shade > 0x80) {
                shade = 0x80;
            }
        } else {
            shade = (4 - object->value) << 5;
            if (shade < 0) {
                shade = 0;
            }
        }
        color = (shade << 24) | 0x808080;
        func_0027E018(0, 0, D_001A6480.x, D_001A6480.y, 0, 0, object->width, object->height, color, D_001C5208);
    } else {
        color = 0x80808080ULL;
        func_0027E018(0, 0, D_001A6480.x, D_001A6480.y, 0, 0, object->width, object->height, color, D_001C5208);
    }
    return 0x10;
}

/* Re-initialises a state block: clears field 68, builds two sub-records from the flag word at field 52 masked with 0x200, substitutes a default record for any that came back zero, then clears field 92, sets fields 84 and 80 to -1 and returns 0. */
s32 func_002DA238 (s32 self)
{
  *(s32 *) (self + 68) = 0;
  *(s32 *) (self + 72) = func_002DE550 (*(s32 *) (self + 52) & 0x200);
  *(s32 *) (self + 76) = func_002DE550 (*(s32 *) (self + 52) & 0x200);
  if ((*(s32 *) (self + 52) & 0x200) == 0)
    {
      if (*(s32 *) (self + 72) == 0)
        *(s32 *) (self + 72) = func_002DE550 (1);
      if (*(s32 *) (self + 76) == 0)
        *(s32 *) (self + 76) = func_002DE550 (1);
    }
  *(s32 *) (self + 92) = 0;
  *(s32 *) (self + 80) = -1;
  *(s32 *) (self + 84) = -1;
  return 0;
}

typedef struct {
    char reserved[0x44];
    int state;
    unsigned int first, second;
    int index, pending;
} Handles_2DCB28;

extern void func_002B7CE0(int);

int func_002DA2D0(Handles_2DCB28 *object) {
    object->first = func_002DE618(object->first);
    object->second = func_002DE618(object->second);
    object->index = -1;
    object->pending = -1;
    object->state = -1;
    func_002B7CE0(1);
    return 0;
}

INCLUDE_ASM("asm/game", func_002DA328);

INCLUDE_ASM("asm/game", func_002DA9A8);

INCLUDE_ASM("asm/game", func_002DAD18);

/* func_002DAE90 is in 002D5E98-g8.c. */

INCLUDE_ASM("asm/game", func_002DAF68);

INCLUDE_ASM("asm/game", func_002DB1B0);

INCLUDE_ASM("asm/game", func_002DB3C0);

INCLUDE_ASM("asm/game", func_002DB648);

INCLUDE_ASM("asm/game", func_002DB7F0);

INCLUDE_ASM("asm/game", func_002DB988);

int func_002DBAA8(void)
{
    extern s32 D_00138344;
    extern s32 D_001F2858;
    extern void func_0025E680(void);
    if (D_00138344 & 0x40)
        D_001F2858 = (s32)func_0025E680;
    return 0;
}

extern void func_00288D88(int, int, int, int);

int func_002DBAD0(char *p) { int x = *(int *)(p + 0x18); int y = *(int *)(p + 0x1C); int w = *(int *)(p + 0x20); int h = *(int *)(p + 0x24); func_00288D88(x, x + w, y, y + h); return 2; }

extern char *D_001F2854;

int func_002DBB08(char *object) {
    *(int *)(D_001F2854 + 0x12C) = 0;
    *(int *)(object + 0x54) = func_002DE550(0);
    return 0;
}

undefined8 func_002DBB48(int param_1)
{
  undefined4 uVar1;
  uVar1 = func_002DE618(*(undefined4 *)(param_1 + 0x54));
  *(undefined4 *)(param_1 + 0x54) = uVar1;
  return 0;
}

extern int D_001A7420 __attribute__((sda));
extern unsigned int D_00138344;
typedef struct { char reserved00[0x14]; char *object; int value; char reserved1C[0xC8]; int clear; } SceneState;
extern SceneState ref_L00_002DBB78_00;
int func_002DBB78(void)
{
    if (D_001A7420 != 16 && D_001A7420 != 1) {
        ref_L00_002DBB78_00.value = *(int *)(ref_L00_002DBB78_00.object + 0xE0);
        return 0;
    }
    {
        unsigned int flags = D_00138344;
        if (flags & 0x20) {
            ref_L00_002DBB78_00.clear = 0;
            ref_L00_002DBB78_00.value = *(int *)(ref_L00_002DBB78_00.object + 0xE0);
            *(int *)(ref_L00_002DBB78_00.object + 0x12C) = 1;
        } else if (flags & 0x10) {
            ref_L00_002DBB78_00.value = *(int *)(ref_L00_002DBB78_00.object + 0xE0);
            *(int *)(ref_L00_002DBB78_00.object + 0x12C) = 0;
        }
    }
    return 0;
}

INCLUDE_ASM("asm/game", func_002DBC10);

#define MACRO_ADDR __attribute__((section(".sdata")))

/* MACRO_ADDR: lui/lw where retail has them, $gp-relative in a delay slot
   (r1_func_0021F7D0, r1_func_002229B0). */
extern int r1_D_0015EE84 MACRO_ADDR;

extern int r1_D_001D48A8[];

/* rac1-decomp src/game/pause.c: func_00222D70 */
/* `r1_D_001D48A8[(unsigned)r1_D_0015EE84 % 19]`; the older near-miss
   predated MACRO_ADDR. */
int func_002DBE80(char *arg0) {
    *(int *)(arg0 + 0x34) = r1_D_001D48A8[(unsigned int)r1_D_0015EE84 % 19];
    return 0;
}

extern int D_00138180[];
extern int D_001F2974;
extern void func_002DF190(int, int);

int func_002DBEC0(char *object)
{
    int flags;
    {
        int *state = D_00138180;
        if (!(state[0x1C4 / 4] & 0x900))
            goto proceed;
        if (D_001F2974 == 0)
            return 1;
    }
proceed:
    {
        int *state = D_00138180;
        flags = state[0x1C4 / 4];
    }
    if (flags & 0x10) {
        char *root = (char *)&D_001F2840;
        int value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                status = -1;
                goto finish;
            default:
                break;
            }
        }
        status = 0;
finish:
        return status;
    }
    {
        int old = *(int *)(object + 0x40);
        if (flags & 0x40)
            *(int *)(object + 0x40) = (old + 1) % 30;
        else if (flags & 0x20)
            *(int *)(object + 0x40) = (old + 29) % 30;
        if (*(int *)(object + 0x40) != old)
            func_002DF190(1, 17);
    }
    return 0;
}


extern int D_00138180[];
extern int D_001F2974;
extern void func_002DF190(int, int);

int func_002DBF98(char *object)
{
    int flags;
    {
        int *state = D_00138180;
        if (!(state[0x1C4 / 4] & 0x900))
            goto proceed;
        if (D_001F2974 == 0)
            return 1;
    }
proceed:
    {
        int *state = D_00138180;
        flags = state[0x1C4 / 4];
    }
    if (flags & 0x10) {
        char *root = (char *)&D_001F2840;
        int value = *(int *)(*(char **)(root + 0x14) + 0xE0);
        int status;
        if (value != 0) {
            *(int *)(root + 0x18) = value;
        } else {
            switch (*(int *)(root + 0x134)) {
            case 0:
                status = -1;
                goto finish;
            default:
                break;
            }
        }
        status = 0;
finish:
        return status;
    }
    if (flags & 0x2040) {
        *(int *)(object + 0x54) = (*(int *)(object + 0x54) + 1) % 12;
        func_002DF190(1, 17);
    } else if (flags & 0x8020) {
        *(int *)(object + 0x54) = (*(int *)(object + 0x54) + 11) % 12;
        func_002DF190(1, 17);
    }
    return 0;
}

/* Two-branch draw set-up: copies a 2-byte record from one of two tables into a 16-byte local, then issues a draw call with five float arguments (scale, per-instance angle, two constants, colour value) plus a 64-bit attribute word, a geometry handle, a fifth integer argument -1 and a local buffer. */
int func_002DC090(char *s0)
{
    signed char local[16];
    float y;

    func_0027C540_1(0);
    if (*(int *)(s0 + 56) != 0) {
        *(TWO_F73af1964 *)local = *(TWO_F73af1964 *)D_L01_001ABE90;
        func_0027F538(4, *(int *)(s0 + 36) / 2 - 8, 0x80FFA888, local, -1);
        y = (float)(*(int *)(s0 + 36) * 8);
        func_0028F410(640.0f, y, 128.0f, 256.0f, 0.0f, 32, 16, func_0028E640(func_0028E588(0xE99D, 6)));
    } else {
        *(TWO_F73af1964 *)local = *(TWO_F73af1964 *)D_L01_001ABE98;
        func_0027F538(*(int *)(s0 + 32) - 24, *(int *)(s0 + 36) / 2 - 8, 0x80FFA888, local, -1);
        y = (float)(*(int *)(s0 + 36) * 8);
        func_0028F410(192.0f, y, 128.0f, 256.0f, 3.1415927f, 32, 16, func_0028E640(func_0028E588(0xE99D, 6)));
    }
    func_0027C660_1();
    return 2;
}


INCLUDE_ASM("asm/game", func_002DC3C8);

INCLUDE_ASM("asm/game", func_002DC720);

INCLUDE_ASM("asm/game", func_002DC900);

/* func_002DCAB8 is built from 002D5E98-g8.c for small-data access. */

int func_002DCB28(Handles_2DCB28 *object) {
    object->first = func_002DE618(object->first);
    return 0;
}

INCLUDE_ASM("asm/game", func_002DCB58);

INCLUDE_ASM("asm/game", func_002DCFE8);

INCLUDE_ASM("asm/game", func_002DD400);

INCLUDE_ASM("asm/game", func_002DDA10);

INCLUDE_ASM("asm/game", func_002DDEB0);
