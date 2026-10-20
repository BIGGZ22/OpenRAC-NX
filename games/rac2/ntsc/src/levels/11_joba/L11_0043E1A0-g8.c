#include "include_asm.h"

typedef int s32;

typedef unsigned int u32;

typedef unsigned char u8;

typedef unsigned short u16;

typedef short s16;

typedef float f32;

/* Small-data controls: the four measured globals the level's script hooks share. */
extern s32 D_L11_001B1530 __attribute__((sda));

extern s32 D_L11_001B1540 __attribute__((sda));

extern s32 D_L11_001B1550 __attribute__((sda));

extern s32 D_L11_001B1560 __attribute__((sda));

extern void func_3004A8(void *);

extern void func_4882F0(void *, void *, s32, s32, s32);

extern void func_488268(s32, s32);

extern void func_4881A8(void *);

extern void func_488200(void *);

extern u32 D_L11_001B32E8[];

extern u8 D_L11_001BA8C0[];

extern u8 *D_L11_001B359C __attribute__((sda));

extern u8 *D_L11_001FC420[];

extern float func_282E48(void *, void *);

extern void func_32C9E0();

extern void func_440C90(u8 *);

/* One class record of the measured class table: six halfwords, then the two flag
   bytes at 0x14 and 0x15.  The record is 26 bytes long. */
typedef struct {
    s16 half[6];
    u8 pad0[8];
    u8 flag14;
    u8 flag15;
    u8 pad1[4];
} JobaClassRecordView;

typedef struct {
    s32 count;
    JobaClassRecordView *records;
} JobaClassTableView;

/* The class table itself: an array of pointers to the per-class entries. */
extern u32 D_L11_001B14F8 __attribute__((sda));

/* Checks the state's six counters against the class record and reports whether the
   class may still be used. */
s32 func_L11_00442710(u8 *obj) {
    u8 *state = *(u8 **)(obj + 0x68);
    s32 index = *(s16 *)(state + 0x1F0);
    s32 cls = *(s16 *)(state + 0x1F2);
    JobaClassTableView *entry = ((JobaClassTableView **)&D_L11_001B14F8)[index];
    JobaClassRecordView *rec = &entry->records[cls];
    u32 *counters = (u32 *)(state + 0x100);
    s32 same = 1;
    s32 i;
    s32 result;
    for (i = 0; i < 6; i++) {
        same &= (counters[i] == (u32)rec->half[i]);
    }
    if (rec->flag14 != 0) {
        same &= *(u32 *)(state + 0x118);
    }
    if (rec->flag15 != 0) {
        same &= *(u32 *)(state + 0x11C);
    }
    result = 0;
    if (*(u32 *)(state + 0x140) == 0) {
        if (same != 0) {
            result = 1;
        }
    }
    return result;
}
/* The four angle and length constants the script reads through $gp. */
extern f32 D_L11_001B15AC __attribute__((sda));

extern f32 D_L11_001B15B0 __attribute__((sda));

extern f32 D_L11_001B15B4 __attribute__((sda));

extern f32 D_L11_001B15B8 __attribute__((sda));

extern float func_00283B98(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern float func_002A7878(float, float);

extern void func_00282CC0(u8 *, u8 *, u8 *);

extern void func_002833F0(u8 *, u8 *);

extern u8 *func_L11_004417E8(u8 *);

extern void func_L11_00436910(char *, int, int);

extern void func_L11_004496A0(char *, int, int);

extern void func_L11_0044C328(char *, int, int);

extern void func_L11_00452C58(char *, int, int);

extern void func_L11_00454C78(char *, int, int);

/* arms the script bridge over the measured buffers and starts it on the shared slot */
void func_L11_00443380(void) {
    s32 buffer[4];
    func_3004A8(buffer);
    func_4882F0(&D_L11_001B1550, &D_L11_001B1560, 1, 0, 0);
    func_488268(2, 0x168);
    func_4881A8(&D_L11_001B1530);
    func_488200(&D_L11_001B1540);
}

/* Turns the script's angle around the actor, hands the result to the level's helpers
   and counts the class the script's identifier names. */
s32 func_L11_00440970(u8 *object, u8 *arg) {
    u8 *state = *(u8 **)(object + 0x68);
    f32 step = D_L11_001B15B4;
    f32 v;

    v = func_00283B98(*(f32 *)(state + 0x19C), step);
    *(f32 *)(state + 0x19C) = v;
    v = func_00283180(v);
    *(f32 *)(arg + 0x10) = D_L11_001B15B8 * v;
    v = func_00283198(*(f32 *)(state + 0x19C));
    *(f32 *)(arg + 0x14) = D_L11_001B15B8 * v;
    func_00282CC0(arg + 0x10, arg + 0x10, func_L11_004417E8(object));
    *(f32 *)(arg + 0x18) = func_002A7878(D_L11_001B15AC, D_L11_001B15B0);
    *(f32 *)(arg + 0xF4) = -1.570796489715576171875f;
    *(s32 *)(arg + 0xF0) = 0;
    *(f32 *)(arg + 0xF8) = *(f32 *)(state + 0x19C);
    func_002833F0(arg + 0xC0, arg + 0xF0);
    switch (*(s16 *)(arg + 0xAA)) {
    case 0xD98:
        func_L11_004496A0(arg, 0, 1);
        *(s32 *)(state + 0x100) += 1;
        break;
    case 0xDCF:
        func_L11_0044C328(arg, 0, 1);
        *(s32 *)(state + 0x104) += 1;
        break;
    case 0xDF4:
        func_L11_00452C58(arg, 0, 1);
        *(s32 *)(state + 0x108) += 1;
        break;
    case 0xDF7:
        func_L11_00454C78(arg, 0, 1);
        *(s32 *)(state + 0x10C) += 1;
        break;
    case 0xD37:
        func_L11_00436910(arg, 0, 1);
        *(s32 *)(state + 0x114) += 1;
        break;
    }
    return 1;
}

