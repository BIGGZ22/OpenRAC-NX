#include "common.h"

/* Plain screen-height global: entry scheduling differs from retail.
 * The worker's exact object used an artificial volatile qualifier; it was rejected.
 */
extern void func_00282CF0(char *, char *, char *);
extern float func_00282DB0(char *, char *);
extern void func_L00_00323F70(void *, int *, int *);
extern int func_L00_002EA668(s32, u16, u16, s32, s32, s32, f32 *);
extern s32 D_001A7340 __attribute__((sda));
extern s32 D_001A7344;
extern f32 D_L04_001B31EC __attribute__((sda));
extern char D_L04_001B9680[];

f32 func_L04_00422A00(u8 *a0) {
    f32 vec[4];
    s32 v10, v14;
    char *s1 = *(char **)(a0 + 0x68) + 0x30;
    char *base = (char *)D_L04_001B9680;
    f32 f20;

    func_00282CF0((char *)vec, s1, base);
    f20 = func_00282DB0((char *)vec, base + 0x230);
    func_L00_00323F70(s1, &v10, &v14);
    if (v10 < 0 || v10 >= D_001A7340 - 0x10 || v14 < 0 || v14 >= D_001A7344 - 8 ||
        !(0.0f < f20)) {
        D_L04_001B31EC = -1.0f;
    } else if (func_L00_002EA668(-1, v10, v14, 0x10, 8, 0, &D_L04_001B31EC) == -1) {
        D_L04_001B31EC = -1.0f;
    }
    return D_L04_001B31EC;
}
