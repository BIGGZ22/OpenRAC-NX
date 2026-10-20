#include "include_asm.h"

extern int ref_L00_00300248_00 __attribute__((sda));
extern unsigned char D_0018C0B0[];
extern void func_002AD250(int, int, int, int);

typedef int s32;

typedef float f32;

/* func_00300048 lives in the boot and in every level, but the vector it scales, the
   four constants it passes and the five functions it calls all sit at a different
   address in every program.  They are therefore named without an address: the
   linker reads their value from the retail bytes of the program it builds.
   The four constants are small data, so they are reached through $gp. */

extern float ref_L00_00300048_06 __attribute__((sda));

extern float ref_L00_00300048_07 __attribute__((sda));

extern float ref_L00_00300048_08 __attribute__((sda));

extern float ref_L00_00300048_09 __attribute__((sda));

extern float ref_L00_00300048_00[];

extern void ref_L00_00300048_01(f32 *, f32 *, f32);

extern void ref_L00_00300048_02(f32 *, f32 *, f32 *);

extern void ref_L00_00300048_03(f32 *, f32, f32);

extern int ref_L00_00300048_04(int, int);

extern void ref_L00_00300048_05(f32 *, f32 *, int, int, int, int, int, int, f32, f32, f32, f32);

void func_00300048(void)
{
    f32 a[4], b[4], c[4], d[4];
    f32 *p = ref_L00_00300048_00;
    f32 *q = p - 140;
    s32 i;
    s32 ret;

    i = 5;
    do {
        ref_L00_00300048_01(a, p, 9.0f);
        ref_L00_00300048_02(b, q, a);
        ref_L00_00300048_03(c, 0.1f, 3.0f);
        ref_L00_00300048_01(a, p, 3.0f);
        ref_L00_00300048_02(d, q, a);
        ref_L00_00300048_02(c, d, c);
        ret = ref_L00_00300048_04(8, 12);
        ref_L00_00300048_05(b, c, ret, 0x00808080, 0x00808080, 0x00FF1010, 0x00FF1010,
                            0x00FF1010, ref_L00_00300048_06, ref_L00_00300048_07,
                            ref_L00_00300048_08, ref_L00_00300048_09);
        i--;
    } while (i >= 0);
}

void func_00300248(int a0, int a1) {
    int state = ref_L00_00300248_00;
    if (state != 0)
        func_002AD250(*(int *)D_0018C0B0, state, a1, a0);
}
