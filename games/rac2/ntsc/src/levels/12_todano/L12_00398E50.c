#include "common.h"

extern u8 D_L12_00188660[];

extern s32 func_315148();

extern void func_31E1D0();

extern s32 func_32D988();

extern s32 func_2F15C8(void *arg0);

extern void func_2F0A60(void *buf, s32 arg1, u32 arg2, s32 arg3, f32 x, f32 y, f32 z);

extern void func_322AF8(void *arg0, void *arg1);

extern void func_326948(void *arg0, void *arg1, s32 arg2, f32 x, f32 y);

extern s32 func_3205F8(void *arg0, s32 arg1, s32 arg2);

extern s32 func_364DF8(s32 arg0, s32 arg1, void *arg2);

extern void func_314008(void *arg0);

extern f32 func_2F2950(f32 x, f32 y);

INCLUDE_ASM("asm/levels", func_L12_00398E50);

INCLUDE_ASM("asm/levels", func_L12_003A0F00);

INCLUDE_ASM("asm/levels", func_L12_003A6E18);

INCLUDE_ASM("asm/levels", func_L12_003A6EF8);

INCLUDE_ASM("asm/levels", func_L12_003A6F48);

INCLUDE_ASM("asm/levels", func_L12_003B5240);

INCLUDE_ASM("asm/levels", func_L12_003BE7D0);


INCLUDE_ASM("asm/levels", func_L12_003CC140);

INCLUDE_ASM("asm/levels", func_L12_003D4110);

INCLUDE_ASM("asm/levels", func_L12_003DE4B0);

INCLUDE_ASM("asm/levels", func_L12_003DF9A0);

INCLUDE_ASM("asm/levels", func_L12_003E04B8);


INCLUDE_ASM("asm/levels", func_L12_003E0858);


INCLUDE_ASM("asm/levels", func_L12_003E0BF0);

INCLUDE_ASM("asm/levels", func_L12_003E1638);


INCLUDE_ASM("asm/levels", func_L12_003E1AA8);

INCLUDE_ASM("asm/levels", func_L12_003E1B28);

INCLUDE_ASM("asm/levels", func_L12_003EB470);

INCLUDE_ASM("asm/levels", func_L12_00402B48);

INCLUDE_ASM("asm/levels", func_L12_00402DF8);

INCLUDE_ASM("asm/levels", func_L12_00404778);


INCLUDE_ASM("asm/levels", func_L12_00412548);

INCLUDE_ASM("asm/levels", func_L12_00412898);


INCLUDE_ASM("asm/levels", func_L12_00412E50);

INCLUDE_ASM("asm/levels", func_L12_00413108);

INCLUDE_ASM("asm/levels", func_L12_004135A8);

INCLUDE_ASM("asm/levels", func_L12_00413CE0);

INCLUDE_ASM("asm/levels", func_L12_00413EA8);


INCLUDE_ASM("asm/levels", func_L12_00414028);

INCLUDE_ASM("asm/levels", func_L12_004141A8);

INCLUDE_ASM("asm/levels", func_L12_00414670);

/* Clears the measured state word and, for the tracked object, the mirror. */
void func_L12_00412DA0(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u8 *tracked;

    *(s32 *)(object + 0x98) = 0;
    tracked = *(u8 **)(state + 0x2F8);
    if (tracked != 0) {
        if (*(s16 *)(tracked + 0xAA) == 0x114B) {
            if (*(u8 *)(tracked + 0x20) != 0xFE) {
                if (*(u8 *)(tracked + 0x20) != 0xFD)
                    *(s32 *)(tracked + 0x98) = 0;
            }
        }
    }
}

/* Copies the tracked object's timer into the state word, for both objects. */
void func_L12_00412DE0(u8 *object) {
    if (object != 0) {
        if (*(u8 *)(object + 0x20) != 0xFE) {
            if (*(u8 *)(object + 0x20) != 0xFD) {
                u8 *tracked;
                *(s32 *)(object + 0x98) = *(s32 *)(*(u8 **)(object + 0x24) + 0x10);
                tracked = *(u8 **)(*(u8 **)(object + 0x68) + 0x2F8);
                if (tracked != 0) {
                    if (*(s16 *)(tracked + 0xAA) == 0x114B) {
                        if (*(u8 *)(tracked + 0x20) != 0xFE) {
                            if (*(u8 *)(tracked + 0x20) != 0xFD)
                                *(s32 *)(tracked + 0x98) = *(s32 *)(*(u8 **)(tracked + 0x24) + 0x10);
                        }
                    }
                }
            }
        }
    }
}

/* Registers the measured two-slot primitive on the given object. */
void func_L12_003E0B68(u8 *object) {
    s32 buffer[4];

    func_315148(object, 2, buffer);
    func_2F0A60(buffer, 0x2E, 0x8080FFFF, 1, 0.25f, 0.0f, 0.0f);
    func_2F0A60(buffer, 0xC, 0x8020F020, 1, 0.25f, 0.0f, 0.0f);
}

/* Advances the measured state machine of the given object. */
void func_L12_004124B0(u8 *object) {
    switch (object[0x20]) {
    case 0:
        if (func_32D988(object) != 0)
            func_31E1D0(object, 1, -1);
        break;
    case 1:
        if (*(f32 *)(object + 0x18) < 160.0f)
            *(f32 *)(object + 0x18) = *(f32 *)(object + 0x18) + 0.0833333358f;
        break;
    }
}

/* Re-arms the measured two-slot primitive after the counter step. */
void func_L12_003E07B0(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 counter;

    func_322AF8(object, state + 0x50);
    func_326948(object, state + 0x6C0, 1, 0.03f, 0.3f);
    func_326948(object, state + 0x640, 0, 0.03f, 0.3f);
    counter = *(u16 *)(object + 0x96);
    if (counter <= 0xFFFE)
        *(u16 *)(object + 0x96) = counter + 1;
}

/* Resets the measured slot state and re-arms the primitive. */
void func_L12_003E19F0(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u16 counter;

    func_322AF8(object, state + 0x50);
    func_2F15C8(state + 0x360);
    func_2F15C8(state + 0x362);
    counter = *(u16 *)(object + 0x96);
    if (counter <= 0xFFFE)
        *(u16 *)(object + 0x96) = counter + 1;
    func_326948(object, state + 0x240, 1, 0.03f, 0.3f);
    func_326948(object, state + 0x2C0, 0, 0.03f, 0.3f);
}

/* Opens the measured window and starts the sub-object. */
void func_L12_003BF7D0(u8 *object) {
    u8 *state = *(u8 **)(object + 0x68);
    u8 *record;
    s32 index;

    if (*(u8 *)(object + 0x20) == 0) {
        *(u8 *)(object + 0x20) = 1;
        *(u8 *)(object + 0x30) = 0x20;
        *(u16 *)(object + 0x32) = 0x20;
        *(s32 *)(state + 0) = -1;
    }
    *(f32 *)(object + 0xF8) = func_2F2950(*(f32 *)(object + 0xF8), 0.017453294f);
    index = *(s32 *)(state + 0);
    record = D_L12_00188660 + index * 0x70;
    if (*(u8 **)(record + 0x88) != object || *(u8 *)(record + 0x74) == 0)
        *(s32 *)(state + 0) = func_364DF8(0, 4, object);
    if (func_3205F8(object, 0x10000, 0) != 0)
        func_314008(object);
}
