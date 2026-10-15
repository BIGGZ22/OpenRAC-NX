#include "include_asm.h"

extern double D_L13_001AEC00 __attribute__((sda));
extern double D_L13_001AEC08 __attribute__((sda));
extern double D_L13_001AEC10 __attribute__((sda));
extern double D_L13_001AEC40 __attribute__((sda));
extern double D_L13_001AEC48 __attribute__((sda));
extern double D_L13_001AEC50 __attribute__((sda));

extern void func_3248E0(unsigned char *, int, int);
extern int func_326C70(unsigned char *, int, int);
extern void func_31A718(unsigned char *);
extern void func_333240(void *, void *, void *, void *, int, int, int, int, int, int, int);
extern void func_3F8C68(void *, int, int, int, int, int, int, int, float, float, int, int, int, int);

void func_L13_0038C718(unsigned char *a0) {
    unsigned char *s1 = *(unsigned char **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        func_3248E0(a0, 1, -1);
        break;
    case 1:
        if (func_326C70(a0, 0x04010000, 0) != 0 && *(int *)s1 == 0)
            func_3248E0(a0, 2, -1);
        a0[0xA8] = 0xFF;
        break;
    case 2:
        func_333240(a0, &D_L13_001AEC10, &D_L13_001AEC08, &D_L13_001AEC00, 0xFFFF, 0, -1, 0,
                    0xFFFF, 0xFF, 0);
        func_31A718(a0);
        break;
    }
}

void func_L13_0038CA20(unsigned char *a0) {
    unsigned char *s1 = *(unsigned char **)(a0 + 0x68);

    switch (a0[0x20]) {
    case 0:
        func_3248E0(a0, 1, -1);
        break;
    case 1:
        if (func_326C70(a0, 0x04010000, 0) != 0 && *(int *)s1 == 0)
            func_3248E0(a0, 2, -1);
        a0[0xA8] = 0xFF;
        break;
    case 2:
        func_333240(a0, &D_L13_001AEC50, &D_L13_001AEC48, &D_L13_001AEC40, 0xFFFF, 0, -1, 0,
                    0xFFFF, 0xFF, 0);
        func_3F8C68(a0 + 0x10, 0, 0x19, 0x14, 0x14, 0x14, 0x2D, 1, 0.3f, 20.0f, 1, 1, 0, 0);
        func_31A718(a0);
        break;
    }
}
