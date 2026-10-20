#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L25_00373ED0);

INCLUDE_ASM("asm/levels", func_L25_00373EE0);

INCLUDE_ASM("asm/levels", func_L25_00373F08);

/* func_L25_003752D8, func_L25_00376FE0 and func_L25_00377608 live in the sibling
   small-data unit (L25_00373ED0-g8.c): their loop counts are read through $gp. */

INCLUDE_ASM("asm/levels", func_L25_003A2378);

INCLUDE_ASM("asm/levels", func_L25_0040CDA0);

INCLUDE_ASM("asm/levels", func_L25_0040D208);

extern char *func_002A9640(char *a0, int a1, int a2);

extern void func_002AB8C0(char *a0, char *a1);

extern void func_002AB800(char *a0, char *a1);

extern int func_002E5DA0(int index, int value, char *object, int kind);

extern void func_L05_00424990(void *a0, void *a1, int a2, int a3, int a4, int a5, int a6, int a7,
                              int a8, int a9, int a10, float f12);

extern void func_L05_00421908(char *a0, int a1, int a2);

extern int func_L05_0037BBB8(char *a0);

extern void func_L05_0037BE98(char *a0);

extern void func_L05_0037AE40(char *a0);

/* The moby's death timer: it ticks the stored time down and, once it reaches zero,
   fires the object's effect and takes the moby down, which it reports. */
int func_L25_0040DCD0(char *obj, int flag)
{
    char *s0;
    float *s2;
    char *s3;
    char *v;
    int ret = 0;

    s3 = func_002A9640(obj, 0x230000, 0);
    *(unsigned char *)(obj + 0xA8) = 0xFF;
    v = *(char **)(obj + 0x68);
    s0 = *(char **)(v + 0x10);
    s2 = *(float **)(v + 0x0);
    if (s0 != 0) {
        func_002AB8C0(obj, s0);
    }
    if (s3 != 0) {
        if (s0 != 0) {
            *(unsigned char *)(s0 + 7) = 0xFF;
            func_002AB800(obj, s0);
        }
        *s2 = *s2 - *(float *)(s3 + 0x2C);
        if (*(short *)(obj + 0xAA) != 0xB73) {
            func_002E5DA0(2, 0, obj, 0x957);
        }
        if (*s2 <= 0.0f) {
            *s2 = 0.0f;
            func_L05_00424990(obj + 0x10, 0, 0x32, 0x1E, 0x1E, 0x1E, 0x28, 1, 1, 0, 0, 0.5f);
            func_L05_00421908(obj, 1, 0x19);
            if (func_L05_0037BBB8(obj)) {
                func_L05_0037BE98(obj);
            }
            if (flag == 0) {
                func_L05_0037AE40(obj);
            }
            ret = 1;
        }
    }
    return ret;
}

INCLUDE_ASM("asm/levels", func_L25_0040DE40);
