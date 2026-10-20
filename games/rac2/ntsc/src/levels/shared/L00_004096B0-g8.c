#include "include_asm.h"

extern void *func_003100D8(int id);

extern int D_L00_001B1340 __attribute__((sda));

/* Allocates a record and tags it released-empty; returns it, or zero when the allocation failed. */
void *func_L00_004115A8(int unused)
{
    char *p = (char *)func_003100D8(0x1136);

    if (p != 0) {
        int v = 0xFF;

        D_L00_001B1340 = (int)p;
        *(unsigned char *)(p + 0x20) = 1;
        *(unsigned short *)(p + 0x32) = v;
        *(unsigned char *)(p + 0x30) = v;
    }
    return p;
}


typedef struct {
    char reserved00[0x18];
    int active;
    char reserved1C[0x14];
} SettingSlot;

typedef struct {
    float amount;
    float scale;
    int kind;
    float multiplier;
} SettingRow;

typedef struct { SettingRow rows[238]; } SettingsBank;

extern SettingSlot D_L00_00256620[30];
extern SettingsBank D_L00_002592C0;

void func_L00_004115E8(void) {
    int i;

    for (i = 29; i >= 0; i--)
        D_L00_00256620[i].active = 0;
    D_L00_001B1340 = 0;
    func_L00_004115A8(0);
    for (i = 119; i < 238; i++) {
        D_L00_002592C0.rows[i - 119].multiplier = 1.0f;
        D_L00_002592C0.rows[i].multiplier = 1.0f;
    }
    D_L00_002592C0.rows[30].amount = 2.0f;
    D_L00_002592C0.rows[30].scale = 0.67f;
    D_L00_002592C0.rows[30].kind = 4;
    D_L00_002592C0.rows[60].amount = 2.0f;
    D_L00_002592C0.rows[60].scale = 0.67f;
    D_L00_002592C0.rows[60].kind = 4;
    D_L00_002592C0.rows[23].amount = 10.0f;
    D_L00_002592C0.rows[23].scale = 5.00f;
    D_L00_002592C0.rows[23].kind = 2;
    D_L00_002592C0.rows[66].amount = 10.0f;
    D_L00_002592C0.rows[66].scale = 5.00f;
    D_L00_002592C0.rows[66].kind = 2;
    D_L00_002592C0.rows[41].amount = 1.0f;
    D_L00_002592C0.rows[41].scale = 2.00f;
    D_L00_002592C0.rows[41].kind = 3;
    D_L00_002592C0.rows[64].amount = 1.0f;
    D_L00_002592C0.rows[64].scale = 2.00f;
    D_L00_002592C0.rows[64].kind = 3;
    D_L00_002592C0.rows[24].amount = 2.0f;
    D_L00_002592C0.rows[24].scale = 2.00f;
    D_L00_002592C0.rows[24].kind = 2;
    D_L00_002592C0.rows[67].amount = 2.0f;
    D_L00_002592C0.rows[67].scale = 2.00f;
    D_L00_002592C0.rows[67].kind = 2;
    D_L00_002592C0.rows[31].amount = 1.0f;
    D_L00_002592C0.rows[31].scale = 1.00f;
    D_L00_002592C0.rows[31].kind = 1;
    D_L00_002592C0.rows[61].amount = 1.0f;
    D_L00_002592C0.rows[61].scale = 1.00f;
    D_L00_002592C0.rows[61].kind = 1;
    D_L00_002592C0.rows[149].amount = 10.0f;
    D_L00_002592C0.rows[149].scale = 4.00f;
    D_L00_002592C0.rows[149].kind = 6;
    D_L00_002592C0.rows[179].amount = 10.0f;
    D_L00_002592C0.rows[179].scale = 4.00f;
    D_L00_002592C0.rows[179].kind = 6;
    D_L00_002592C0.rows[142].amount = 10.0f;
    D_L00_002592C0.rows[142].scale = 8.00f;
    D_L00_002592C0.rows[142].kind = 3;
    D_L00_002592C0.rows[185].amount = 10.0f;
    D_L00_002592C0.rows[185].scale = 8.00f;
    D_L00_002592C0.rows[185].kind = 3;
    D_L00_002592C0.rows[160].amount = 2.0f;
    D_L00_002592C0.rows[160].scale = 4.00f;
    D_L00_002592C0.rows[160].kind = 3;
    D_L00_002592C0.rows[183].amount = 2.0f;
    D_L00_002592C0.rows[183].scale = 4.00f;
    D_L00_002592C0.rows[183].kind = 3;
    D_L00_002592C0.rows[143].amount = 6.0f;
    D_L00_002592C0.rows[143].scale = 2.00f;
    D_L00_002592C0.rows[143].kind = 3;
    D_L00_002592C0.rows[186].amount = 6.0f;
    D_L00_002592C0.rows[186].scale = 2.00f;
    D_L00_002592C0.rows[186].kind = 3;
    D_L00_002592C0.rows[150].amount = 1.0f;
    D_L00_002592C0.rows[150].scale = 1.00f;
    D_L00_002592C0.rows[150].kind = 1;
    D_L00_002592C0.rows[180].amount = 1.0f;
    D_L00_002592C0.rows[180].scale = 1.00f;
    D_L00_002592C0.rows[180].kind = 1;
}
