#include "common.h"

extern u8 *D_L12_001B2E9C __attribute__((sda));

extern u16 *D_L12_001FB960[];

extern s32 D_L12_001B09F4 __attribute__((sda));

extern s32 func_31DEF0(void *arg0, s32 arg1, s32 arg2, s32 arg3);

extern void func_31E1D0(void *arg0, s32 arg1, s32 arg2);

extern void func_2F1958(f32 *buf, u8 *arg1, u8 *arg2);

extern void func_323F98(f32 *buf, f32 x);

extern void func_2F1928(u8 *arg0, u8 *arg1, f32 *buf);

extern void func_321940(u8 *arg0, s32 arg1, f32 x, f32 y);

extern void func_3239B8(void *arg0, void *arg1, s32 arg2, f32 x, f32 y);

extern s32 D_L12_001B1838 __attribute__((sda));

extern s32 D_L12_001B183C __attribute__((sda));

extern s32 D_L12_001B1840 __attribute__((sda));

extern u16 D_L12_001B1834 __attribute__((sda));

extern f32 D_L12_001B1858 __attribute__((sda));

extern s32 func_2F15C8(void *arg0);

extern s32 func_31E2E0(s32 arg0, s32 arg1);

extern void func_3296A8(void *arg0, void *arg1, s32 arg2, f32 x, s32 arg3);

/* Walks the measured table of the given index down to its terminator. */
s32 func_L12_003E09C8(s32 index) {
    u16 *entry = D_L12_001FB960[index];
    u16 packed;
    u8 *base;

    if (entry == 0)
        return 1;
    base = D_L12_001B2E9C;
    do {
        u8 *record;
        packed = *entry;
        record = base + (packed & 0x7FFF) * 0x100;
        if (*(s16 *)(record + 0xAA) == 0xBB8) {
            if (*(u8 *)(record + 0x20) != 1)
                return 0;
        }
        entry++;
    } while ((s16)packed >= 0);
    return 1;
}

/* Arms the measured sub-object on the first entry. */
void func_L12_003E0A38(u8 *object, u8 *arg1, u8 *arg2, s32 arg3) {
    f32 buffer[4];
    u16 flags;

    if (*(u8 *)(object + 0x43) != 0xA)
        func_31DEF0(object, 0xA, 0, 5);
    if (*(u8 *)(object + 0xBC) != 8)
        func_31E1D0(object, 8, 1);
    func_2F1958(buffer, arg1, object + 0x10);
    func_323F98(buffer, 0.16666667f);
    func_2F1928(object + 0x10, object + 0x10, buffer);
    func_321940(object + 0xF8, 0, *(f32 *)(arg2 + 8), 0.20943954f);
    if (arg3 != 0) {
        flags = *(u16 *)(object + 0x34);
        D_L12_001B09F4 = D_L12_001B09F4 + 1;
        *(s32 *)(object + 0x98) = 0;
        *(u16 *)(object + 0x34) = (flags | 0x41) & 0xEFFF;
        func_3239B8(object, object + 0x10, -1, 1.0f, 0.0f);
        func_31E1D0(object, 1, 0);
    }
}

/* Steps the measured slot timer and restarts the tracked sub-object. */
void func_L12_00413F68(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 counter;

    if (*(u8 *)(state + 0x2EA) != 0) {
        *(u8 *)(state + 0x2EA) = 0;
        if (*(s16 *)(state + 0x2E0) < D_L12_001B1838)
            *(u16 *)(state + 0x2E0) = func_31E2E0(D_L12_001B183C, D_L12_001B1840);
    }
    if (func_2F15C8(state + 0x2E0) == 0)
        *(u16 *)(state + 0x2E8) = D_L12_001B1834;
    func_2F15C8(state + 0x2E2);
    func_2F15C8(state + 0x2E6);
    func_2F15C8(state + 0x2E8);
    counter = *(u16 *)(object + 0x96);
    if (counter <= 0xFFFE)
        *(u16 *)(object + 0x96) = counter + 1;
    func_3296A8(*(void **)(state + 0x2F8), state + 0x420, 3, D_L12_001B1858, 1);
}

extern u8 *D_L12_001B090C __attribute__((sda));

extern u8 *D_L12_001B1360 __attribute__((sda));

/* Whether the tracked object is in a state that still counts as open. */
s32 func_L12_003DE4B0(void) {
    u8 *object = D_L12_001B090C;
    s32 result = 1;

    if (object == 0) {
        result = 0;
    } else {
        if (*(u8 *)(object + 0x20) == 6 || *(s8 *)(object + 0x20) < 0
            || *(u8 *)(object + 0x20) == 8)
            result = 0;
    }
    return result;
}

/* The same test, with one more state in the closed set. */
s32 func_L12_00404778(void) {
    u8 *object = D_L12_001B1360;
    s32 result = 1;

    if (object == 0) {
        result = 0;
    } else {
        if (*(u8 *)(object + 0x20) == 6) {
            result = 0;
        } else if (*(u8 *)(object + 0x20) == 5 || *(u8 *)(object + 0x20) == 8
                 || *(s8 *)(object + 0x20) < 0) {
            result = 0;
        }
    }
    return result;
}
