#include "include_asm.h"
#include "common.h"

extern int D_L00_001B1B30;
extern int D_L00_001A9034 __attribute__((sda));
extern int D_L00_001B1B48;
extern int D_L00_001A9038 __attribute__((sda));

void func_L00_0032A138(void) {
    int count = 4;
    int *cursor = &D_L00_001B1B30;

    do {
        *cursor = 0;
        --cursor;
        --count;
    } while (count >= 0);
    D_L00_001A9034 = 0;
}

void func_L00_0032A170(void) {
    int count = 4;
    int *cursor = &D_L00_001B1B48;

    do {
        *cursor = 0;
        --cursor;
        --count;
    } while (count >= 0);
    D_L00_001A9038 = 0;
}

extern int D_L00_001A8FB4;
extern int D_L00_001A8FB4_1 __attribute__((sda));
extern int D_L00_001A8FB0;
extern int D_L00_001B2084;
extern int D_L00_001B2084_1 __attribute__((sda));
extern void func_L00_0030DDF0(void);
extern void func_0028B5D0(int);

int func_L00_003268E8(int value) {
    if (D_L00_001A8FB4 != value)
        return 0;
    if (D_L00_001B2084 == -1)
        return 0;

    D_L00_001A8FB0 = 0;
    D_L00_001A8FB4_1 = 0;
    func_L00_0030DDF0();
    func_0028B5D0(D_L00_001B2084_1);
    D_L00_001B2084 = -1;
    return 1;
}

extern unsigned char *D_L00_001B1F9C;

extern int func_2EE138(void *, void *, void *);
extern void func_2EE5B0(void *, void *, void *);
extern float func_2EE268(void *);

/* Screens the 128-byte record the index picks out of the resident object table: runs the
   caller's transform through the record's two matrices and accepts only when the remaining
   length is under 1 and the record's own z direction stays inside [-1, 1]. */
int func_L00_003206B8(void *a0, int a1)
{
    unsigned char *s0;
    float b1[4];
    float b2[4];

    if (a1 < 0)
        return 0;
    s0 = D_L00_001B1F9C + (a1 << 7);
    func_2EE138(b1, a0, s0 + 0x30);
    *(int *)((unsigned char *)b1 + 0xC) = 0;
    func_2EE5B0(b2, b1, s0 + 0x40);
    if (func_2EE268(b2) < 1.0f) {
        if (*(float *)((unsigned char *)b2 + 8) >= -1.0f) {
            if (*(float *)((unsigned char *)b2 + 8) <= 1.0f)
                return 1;
        }
    }
    return 0;
}
