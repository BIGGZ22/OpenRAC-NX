#include "include_asm.h"
typedef unsigned char u8;

extern int func_00343060(u8 *);
extern void func_00335E68(u8 *, int);
extern void func_00335E38(void *, float, float, float, float);
extern float D_001AE214;
extern float D_001AE210 __attribute__((sda));
void func_003435C8(u8 *a0) {
    if (func_00343060(a0 + 0x298) == 0) {
        func_00335E68(a0 + 0x1F8, 0);
    } else {
        int *v = *(int **)(a0 + 0x288);
        func_00335E38(a0 + 0x1F8, D_001AE210 + *(float *)v,
                      D_001AE214 + *(float *)((u8 *)v + 4), 0.0f, 0.0f);
    }
}

extern const char *func_00288FB8(int);
extern void func_00115DA8(char *, const char *, ...);
extern unsigned int func_001157AC(const char *);
extern void func_00115AC0(void *, int, int);
extern char D_001AE220[];
extern char D_001ADBC8[];
extern float D_001AE218 __attribute__((sda));
extern float D_001AE21C;
void func_00343638(char *object)
{
    char buffer[256];
    float *position;
    func_00335E68((u8 *)object + 0x1B0, 1);
    switch (*(int *)(object + 0x294)) {
    case 0:
        func_00115DA8(buffer, D_001AE220, func_00288FB8(0x2D51), func_00288FB8(0x2D55));
        break;
    case 2:
        func_00115DA8(buffer, D_001AE220, func_00288FB8(0x2D54), func_00288FB8(0x2D55));
        break;
    case 1:
        func_00115DA8(buffer, D_001AE220, func_00288FB8(0x2D52), func_00288FB8(0x2D55));
        break;
    }
    if (func_001157AC(buffer) >= 49)
        func_00115DA8(object + 0x120, D_001ADBC8, func_00288FB8(0));
    else
        func_00115AC0(object + 0x120, (int)buffer, 49);
    position = *(float **)(object + 0x288);
    func_00335E38(object + 0x1B0, D_001AE218 + position[0], D_001AE21C + position[1], 0.0f, 0.0f);
}
