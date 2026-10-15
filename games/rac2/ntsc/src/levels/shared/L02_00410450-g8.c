#include "include_asm.h"

typedef unsigned char u8;

extern float D_L02_001B1FC4 __attribute__((sda));

extern void func_002F2270(void *, void *, float);
extern void func_00322F98(void *, void *, float);
extern void func_002F1FF8(void *, void *, float);
extern void func_00324C28(void *, void *, void *);
extern void func_002F1F70(void *, void *, void *);
extern float func_002F3048(float, float);
extern void func_002EA180(void *, void *);
extern void func_L02_00422638(int);

/* Rebuilds the record's second frame and folds the resident angle step in. */
void func_L02_00422538(char *s2) {
    float buf[4];
    char *s0 = *(char **)(s2 + 0x68);
    char *s3 = s2 + 0x10;
    char *s1 = s0 + 0x10;

    func_002F2270(buf, s0 + 0x40, 1.0f);
    func_00322F98(buf, buf, *(float *)(s0 + 0x30));
    func_002F1FF8(s1, s0, *(float *)(s0 + 0x60));
    func_00324C28(s1, s1, buf);
    *(float *)(s0 + 0x20) = -*(float *)(s0 + 0x10);
    *(float *)(s0 + 0x24) = -*(float *)(s0 + 0x14);
    *(float *)(s0 + 0x28) = -*(float *)(s0 + 0x18);
    func_002F1F70(s1, s3, s1);
    func_002F1F70(s0 + 0x20, s3, s0 + 0x20);
    *(float *)(s0 + 0x30) =
        func_002F3048(*(float *)(s0 + 0x30), D_L02_001B1FC4 * 0.000290888245f);
    func_002EA180(func_L02_00422638, s2);
}
