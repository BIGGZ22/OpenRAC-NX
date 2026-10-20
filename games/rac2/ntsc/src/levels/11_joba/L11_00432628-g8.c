#include "include_asm.h"

#include "common.h"

/* Small-data unit: the bodies whose state lives at $gp-relative offsets. */

extern float D_L11_001B12C8 __attribute__((sda));

/* Bleeds the ten rotation timers the level keeps beside its state: each armed
   entry loses a sixtieth of the shared rate and clamps at zero, disarming. */
INCLUDE_ASM("asm/levels", func_L11_004395C8);

extern float D_L11_001B1338 __attribute__((sda));

extern void func_002A7470(void *, int, int, int);
extern float func_002A7878(float, float);
extern int func_002A77E0(int);
extern void func_00282A60(unsigned int *, unsigned int, int);
extern void func_0029F978(unsigned char *, unsigned int, unsigned char *);

/* Re-aims the object: resets the measured state, draws a random angle and
   clears the two rotation slots of the state. */
void func_L11_0043A9D8(char *obj)
{
    char *state = *(char **)(obj + 0x68);
    float f20;
    float f0;

    *(unsigned char *)(state + 0x2E) = 3;
    *(float *)(state + 0x20) = (float)*(short *)(state + 0x24);
    *(short *)(state + 0x1A8) = 0;
    *(unsigned char *)(state + 0x18A) = 0;
    *(unsigned char *)(obj + 0x31) = 1;
    *(unsigned short *)(obj + 0x34) &= 0xFFFE;
    *(int *)(obj + 0x98) = *(int *)(*(int *)(obj + 0x24) + 0x10);
    *(unsigned short *)(obj + 0x34) = (*(unsigned short *)(obj + 0x34) | 0x1000) & 0xFFB9;
    *(unsigned char *)(obj + 0x7F) = 0;
    if (*(unsigned char *)(obj + 0x43) != 1)
        func_002A7470(obj, 1, 0, 0);
    f20 = func_002A7878(10.0f, 30.0f) * 0.017453294f;
    if (func_002A77E0(2) != 0)
        f20 = -f20;
    *(float *)(state + 0x2D4) = f20;
    if (func_002A77E0(2) != 0)
        f0 = -D_L11_001B1338;
    else
        f0 = D_L11_001B1338;
    *(short *)(state + 0x2EE) = 0;
    *(float *)(state + 0x2DC) = f0;
    func_00282A60((unsigned int *)(state + 0x220), 0, 0x40);
    func_0029F978((unsigned char *)obj, 2, (unsigned char *)(state + 0x220));
    func_00282A60((unsigned int *)(state + 0x260), 0, 0x40);
    func_0029F978((unsigned char *)obj, 3, (unsigned char *)(state + 0x260));
}
