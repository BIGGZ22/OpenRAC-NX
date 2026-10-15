#include "include_asm.h"
extern int func_002ED688(unsigned char *);
extern float func_00282DF0(void *);
extern void func_00282F20(void *, void *, float);
extern float D_L00_001AC588 __attribute__((sda));

void func_002EE610(unsigned char *owner)
{
    char *data = (char *)func_002ED688(owner);
    if (data) {
        char *vector = data + 0x90;
        float length = func_00282DF0(vector);
        float limit = D_L00_001AC588;
        *(float *)(data + 0x124) = length;
        if (limit < length) {
            func_00282F20(vector, vector, limit);
            *(float *)(data + 0x124) = D_L00_001AC588;
        }
    }
}
