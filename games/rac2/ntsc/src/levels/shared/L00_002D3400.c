#include "include_asm.h"

typedef unsigned char u8;

typedef int s32;

typedef struct {
    u8 field00[0x218]; signed short field218; u8 field21A[0x12e];
    signed short field348; u8 field34A[4]; signed short field34E;
    u8 field350[0x1147]; u8 field1497; u8 field1498[2]; u8 field149A;
    u8 field149B[0xdf9]; s32 field2294;
} NativeModeResident;

extern NativeModeResident D_00189E20_8;

extern s32 D_L00_001A8FF0 __attribute__((sda));

extern volatile u8 D_001A7B08[4] __attribute__((sda));

extern s32 D_L00_001A8FF4_1 __attribute__((sda));

typedef float f32;

#ifndef RAC2_T_S16_FC94396F4
#define RAC2_T_S16_FC94396F4
typedef signed short s16_Fc94396f4;
#endif

#ifndef RAC2_T_FAMILYC94396F4VECTOR_FC94396F4
#define RAC2_T_FAMILYC94396F4VECTOR_FC94396F4
typedef struct {
    f32 x;
    f32 y;
    f32 z;
} FamilyC94396F4Vector_Fc94396f4;
#endif

#ifndef RAC2_T_FAMILYC94396F4STATE_FC94396F4
#define RAC2_T_FAMILYC94396F4STATE_FC94396F4
typedef struct {
    u8 reserved0[846];
    s16_Fc94396f4 gate;
    u8 reserved1[8004];
    s32 state;
    u8 reserved2[43];
    u8 mode;
} FamilyC94396F4State_Fc94396f4;
#endif

extern FamilyC94396F4State_Fc94396f4 D_00189E20_15;

extern u8 D_0018A0E0[];

extern void func_00282F20_6(u8 *buffer, u8 *table, f32 parameter);

extern void func_L00_002D3BD8(u8 *buffer, FamilyC94396F4Vector_Fc94396f4 *source, f32 zero);

extern void func_L00_002D3B58(FamilyC94396F4Vector_Fc94396f4 *destination,
                    FamilyC94396F4Vector_Fc94396f4 *source, f32 zero);

extern void func_00282CC0_16(FamilyC94396F4Vector_Fc94396f4 *destination,
                    FamilyC94396F4Vector_Fc94396f4 *source, u8 *buffer);

extern float func_00283180_1(float v);

extern float func_00283198_1(float v);

extern void func_002830C0_2(void *p, void *q, char *data);

extern void func_00282CC0_2(void *p, void *q, char *data);

extern char D_00189E20_6[];

/* Measured partial layouts; original type and game roles are unknown. */
typedef float Rac2Native_5fc519c90e0e763a_f32;

extern void func_L00_002D34C0_1(Rac2Native_5fc519c90e0e763a_f32);

/* resident mode predicate: returns 0, 1 or 2 from the resident state words */
s32 func_L00_002D3400(void) {
    s32 result = D_00189E20_8.field348;
    if (D_L00_001A8FF0 != 0 && D_L00_001A8FF4_1 == 0) result = 2;
    if (D_L00_001A8FF4_1 != 0 || D_00189E20_8.field2294 == 59 ||
        (D_00189E20_8.field2294 == 110 && D_00189E20_8.field149A == 0) ||
        D_00189E20_8.field2294 == 109 || D_00189E20_8.field218 != 0 ||
        (D_00189E20_8.field1497 != 0 && D_001A7B08[3] != 0 &&
         D_00189E20_8.field2294 == 0 && D_00189E20_8.field34E < 4)) result = 1;
    if (result == 1 && D_00189E20_8.field149A != 0) result = 0;
    return result;
}

/* Dispatches a shared mode to update the destination Z component or compose two temporary transforms through the measured helper sequence. */
void func_L00_002D34C0(FamilyC94396F4Vector_Fc94396f4 *destination,
                                      FamilyC94396F4Vector_Fc94396f4 *source,
                                      f32 parameter)
{
    u8 first[16];
    u8 second[16];

    switch (D_00189E20_15.mode) {
    case 0:
        destination->z = source->z - parameter;
        return;
    case 1:
        func_00282F20_6(first, (u8 *)&D_00189E20_15 + 672, -parameter);
        func_L00_002D3BD8(second, source, 0.0f);
        func_L00_002D3B58(destination, destination, 0.0f);
        func_00282CC0_16(destination, destination, second);
        func_00282CC0_16(destination, destination, first);
        return;
    case 2:
        if (D_00189E20_15.state == 58 && D_00189E20_15.gate == 0) {
            func_00282F20_6(first, (u8 *)&D_00189E20_15 + 672, -parameter);
        } else {
            func_00282F20_6(first, D_0018A0E0, parameter);
        }
        func_L00_002D3BD8(second, source, 0.0f);
        func_L00_002D3B58(destination, destination, 0.0f);
        func_00282CC0_16(destination, destination, second);
        func_00282CC0_16(destination, destination, first);
        return;
    default:
        return;
    }
}

/* Fills a three-float record from two helper results scaled by a factor and a third float argument, then runs a setup routine on the record with a bound data root and, when the flag argument is non-zero, a second routine with that root plus 128. */
void func_L00_002D3638(float *p, int flag, float scale, float value, float height)
{
    p[0] = func_00283180_1(value) * scale;
    p[1] = func_00283198_1(value) * scale;
    p[2] = height;
    func_002830C0_2(p, p, D_00189E20_6);
    if (flag != 0)
        func_00282CC0_2(p, p, D_00189E20_6 + 128);
}

INCLUDE_ASM("asm/levels", func_L00_002D36E0);

/* calls the measured float callee with the negated input */
void func_L00_002D3778(Rac2Native_5fc519c90e0e763a_f32 value) {
 func_L00_002D34C0_1(-value);
}
