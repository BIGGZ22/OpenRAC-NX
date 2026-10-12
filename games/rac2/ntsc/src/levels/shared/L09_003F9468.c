#include "include_asm.h"

/* Update two linked parts from a valid parent and propagate object state and color. */
typedef struct DobboPartsObject252 DobboPartsObject252;

typedef struct DobboPartsDescriptor252 {
    unsigned char gap0[0x2f0];
    DobboPartsObject252 *parts[2];
    DobboPartsObject252 *parent;
} DobboPartsDescriptor252;

struct DobboPartsObject252 {
    unsigned char gap0[0x20];
    unsigned char status;
    unsigned char gap21[0x10];
    unsigned char disabled;
    unsigned char gap32[2];
    unsigned short flags;
    unsigned char gap36[0x32];
    DobboPartsDescriptor252 *descriptor;
    unsigned char gap6c[0x3e];
    short class_code;
};

extern void func_002AF1D0(void *, void *, int, int);

extern void func_L09_003F7788(void *, void *);

typedef float f32;

typedef unsigned char u8;

typedef int s32;

typedef struct {
    u8 field00[0x1f0];
    f32 field1F0[4];
    u8 field200[0xe0];
    f32 field2E0;
    u8 field2E4[4];
    f32 field2E8;
} NativeAngleState __attribute__((aligned(16)));

typedef struct {
    u8 field00[0x10];
    f32 field10[4];
    u8 field20[0x48];
    NativeAngleState *field68;
} NativeAngleOwner __attribute__((aligned(16)));

extern void func_00282CF0(f32 *output, const f32 *left, const f32 *right);

extern void func_002AFDD8(NativeAngleOwner *owner, f32 *output, const f32 *input, s32 mode);

extern f32 func_00283248(f32 x, f32 y);

extern f32 func_00282E20(const f32 *vector);

extern void func_00282CF0(float *, const float *, const float *);

/* validate descriptorparent classD0E/status then attach two optionalparts, propagate flag/disabled state and original packed-color helper */
void func_L09_003F9468(DobboPartsObject252 *object)
{
    DobboPartsDescriptor252 *descriptor = object->descriptor;
    DobboPartsObject252 *parent = descriptor->parent;
    int i;
    if (!parent || parent->class_code != 0xd0e || parent->status == 0xfe || parent->status == 0xfd)
        return;
    for (i = 0; i < 2; ++i) {
        if (descriptor->parts[i]) {
            func_002AF1D0(descriptor->parent, descriptor->parts[i], i, i == 1 ? 6 : 0);
            if (object->disabled) {
                descriptor->parts[i]->flags &= ~1U;
                descriptor->parts[i]->disabled = 1;
            } else {
                descriptor->parts[i]->flags |= 1;
                descriptor->parts[i]->disabled = 0;
            }
            func_L09_003F7788(object, descriptor->parts[i]);
        }
    }
}

INCLUDE_ASM("asm/levels", func_L09_003F9568);

INCLUDE_ASM("asm/levels", func_L09_003F9708);

INCLUDE_ASM("asm/levels", func_L09_003F9928);

INCLUDE_ASM("asm/levels", func_L09_003F9970);

INCLUDE_ASM("asm/levels", func_L09_003F9A70);

INCLUDE_ASM("asm/levels", func_L09_003F9D08);

INCLUDE_ASM("asm/levels", func_L09_003FA080);

/* computes a local target direction and stores bounded horizontal and vertical angles through the original math helpers */
void func_L09_004051F8(NativeAngleOwner *owner) {
    f32 direction[4] __attribute__((aligned(16)));
    NativeAngleState *state = owner->field68;
    f32 angle, xy_length;
    func_00282CF0(direction, state->field1F0, owner->field10);
    func_002AFDD8(owner, direction, direction, 0);
    angle = func_00283248(direction[0], direction[1]);
    state->field2E0 = angle;
    if (0.78539824f < angle) state->field2E0 = 0.78539824f;
    else if (angle < -0.78539824f) state->field2E0 = -0.78539824f;
    xy_length = func_00282E20(direction);
    angle = -func_00283248(xy_length, direction[2]);
    state->field2E8 = angle;
    if (0.52359885f < angle) state->field2E8 = 0.52359885f;
    else if (angle < -0.52359885f) state->field2E8 = -0.52359885f;
}

INCLUDE_ASM("asm/levels", func_L09_00410200);

INCLUDE_ASM("asm/levels", func_L09_00410608);

INCLUDE_ASM("asm/levels", func_L09_00410B40);

INCLUDE_ASM("asm/levels", func_L09_00410BC8);

INCLUDE_ASM("asm/levels", func_L09_0041A700);
