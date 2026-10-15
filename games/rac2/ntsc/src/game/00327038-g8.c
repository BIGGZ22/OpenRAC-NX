#include "include_asm.h"

/* The threshold lives in small data, so this file is built with -G8. It differs
   from program to program, so it carries no level. */
extern float ref_L00_00327D08_00 __attribute__((sda));
extern void func_002FC808(void);
extern void func_00327D50(char *);

/* Runs the shared update, then advances the object's sub-record while its
   stored float is still below the small-data threshold. */
void func_00327D08(char *a0)
{
    float value;

    func_002FC808();
    value = *(float *)(*(char **)(a0 + 0x68) + 0x24);
    if (value < ref_L00_00327D08_00)
        func_00327D50(a0);
}
