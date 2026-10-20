#include "include_asm.h"

extern char *func_00288FB8(int);
extern void func_00336A38(void *, char *);
extern void func_00115DA8(char *, const char *, ...);
extern void func_00336A30(void *, int);
extern void func_00336A78(void *);
extern int func_002B0DB8(void);
extern int func_002B0D60(void);
extern int func_002B0D70(void);
extern float *func_00335E10(void *);
extern void func_00335E38(void *, float, float, float, float);
extern void func_00335E68(void *, int);
extern void func_0028FC48(int, int, int, int, unsigned long, int);
extern char D_001ADBC8[], D_001ADF98[], D_001AE2F8[];
extern short D_001A7A1A;
extern int D_001A7A1C;
extern int D_001A79FC;
extern int D_001AE2F0 __attribute__((sda));
extern int D_001AE2E0 __attribute__((sda));
extern int D_001AE2E4;
extern int D_001AE2E8 __attribute__((sda));
extern int D_001AE2EC;

void func_00344938(char *self)
{
    int sel = 0;
    float zero;

    if (*(int *)(self + 0x448) == 3)
        sel = *(int *)(self + 0x43C) + *(int *)(self + 0x440) * 2;
    self[0x410] = 0;
    self[0x420] = 0;
    func_00336A38(self + 0x2E0, func_00288FB8(0xAD2));
    switch (sel) {
    case 0:
        func_00336A38(self + 0x210, func_00288FB8(0xAD2));
        zero = 0.0f;
        func_00115DA8(self + 0x400, D_001ADBC8, func_00288FB8(0x2DBB));
        func_00336A30(self + 0x328, 0);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x400, D_001ADF98, func_002B0DB8());
        func_00335E38(self + 0x328, func_00335E10(self + 0x328)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x328)[1], zero, zero);
        func_00336A30(self + 0x328, 2);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x410, D_001ADBC8, func_00288FB8(0x2DBC));
        func_00336A30(self + 0x370, 0);
        func_00336A78(self + 0x370);
        func_00115DA8(self + 0x410, D_001ADF98, func_002B0D60());
        func_00335E38(self + 0x370, func_00335E10(self + 0x370)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x370)[1], zero, zero);
        func_00336A30(self + 0x370, 2);
        func_00336A78(self + 0x370);
        func_00115DA8(self + 0x420, D_001ADBC8, func_00288FB8(0x2DBD));
        func_00336A30(self + 0x3B8, 0);
        func_00336A78(self + 0x3B8);
        func_00115DA8(self + 0x420, D_001ADF98, func_002B0D70());
        func_00335E38(self + 0x3B8, func_00335E10(self + 0x3B8)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x3B8)[1], zero, zero);
        func_00336A30(self + 0x3B8, 2);
        func_00336A78(self + 0x3B8);
        break;
    case 1:
        func_00115DA8(self + 0x400, D_001ADBC8, func_00288FB8(0x2DBB));
        func_00336A30(self + 0x328, 0);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x400, D_001ADF98, D_001A7A1A);
        func_00335E38(self + 0x328, func_00335E10(self + 0x328)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x328)[1] + 16.0f, 0.0f, 0.0f);
        func_00336A30(self + 0x328, 2);
        func_00336A78(self + 0x328);
        func_00336A38(self + 0x2E0, func_00288FB8(0xAD1));
        func_00336A38(self + 0x210, func_00288FB8(0xAD1));
        break;
    case 2:
        func_00115DA8(self + 0x400, D_001ADBC8, func_00288FB8(0x2DBB));
        func_00336A30(self + 0x328, 0);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x400, D_001ADF98, D_001A7A1C);
        func_00335E38(self + 0x328, func_00335E10(self + 0x328)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x328)[1] + 16.0f, 0.0f, 0.0f);
        func_00336A30(self + 0x328, 2);
        func_00336A78(self + 0x328);
        func_00336A38(self + 0x2E0, func_00288FB8(0xAD0));
        func_00336A38(self + 0x210, func_00288FB8(0xAD0));
        break;
    case 3:
        func_00115DA8(self + 0x400, D_001ADBC8, func_00288FB8(0x2DBB));
        func_00336A30(self + 0x328, 0);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x400, D_001ADF98, D_001A79FC);
        func_00335E38(self + 0x328, func_00335E10(self + 0x328)[0] + (float)D_001AE2F0,
                      func_00335E10(self + 0x328)[1] + 16.0f, 0.0f, 0.0f);
        func_00336A30(self + 0x328, 2);
        func_00336A78(self + 0x328);
        func_00115DA8(self + 0x400, D_001AE2F8, func_00288FB8(0x2DBB), D_001A79FC);
        func_00336A38(self + 0x2E0, func_00288FB8(0xACF));
        func_00336A38(self + 0x210, func_00288FB8(0xACF));
        break;
    }
    func_00336A78(self + 0x2E0);
    if (*(int *)(self + 0x444) == sel) {
        func_00335E68(self + 0x210, 1);
        func_00335E38(self + 0x210, (*(float **)(self + 0x430))[0] + -117.0f,
                      (*(float **)(self + 0x430))[1] + 3.0f, 0.0f, 0.0f);
        func_00336A78(self + 0x210);
    }
    if (sel == 0) {
        int x = (*(float **)(self + 0x430))[0];
        int y = (*(float **)(self + 0x430))[1];
        func_0028FC48(x + D_001AE2E0, y + D_001AE2E4, x + D_001AE2E8, y + D_001AE2EC,
                      0x55F0C070, 0);
    }
}
