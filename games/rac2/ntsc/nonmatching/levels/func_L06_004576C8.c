/* Correct table extents retained; the small-data address expands by eight bytes.
 * Independent check in the original G8 unit: 0 of 1 places. */
#include "include_asm.h"
#include "common.h"

extern char *D_L06_001B281C __attribute__((sda));

extern u8 *D_L06_001FB3A0[];

/* The sixteen angles the eight pairs of the level's table hold, in degrees: the
   table is small data, so its address is taken $gp-relative. */
extern float D_L06_001B1890[16] __attribute__((sda));
/* The two globals the moby list is built from: they are addressed absolutely. */
extern char D_L06_001A8F20[0x20] __attribute__((sda));
extern char D_L06_001A8F40[0x20] __attribute__((sda));

extern float func_00283C28(float);
extern void func_002AF000(void *, float, float, float);
extern int func_L03_0042B6F8(void *, void *, int, void *, int, int);

void func_L06_0041F798(u8 *arg0)
{
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L06_001FB3A0[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = D_L06_001B281C;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xBBA) {
                *(int *)(*(char **)(q + 0x68) + 0x258) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

void func_L06_00443E88(u8 *arg0)
{
    char *a2;
    u8 *p;
    u8 *list;

    if (arg0[0x21] == 0xFF)
        return;
    list = D_L06_001FB3A0[arg0[0x21]];
    if (list == 0)
        return;
    p = list;
    a2 = D_L06_001B281C;
    for (;;) {
        char *q = a2 + ((*(unsigned short *)p & 0x7FFF) << 8);

        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xDB0) {
                *(short *)(*(int *)(q + 0x68) + 0x316) = 1;
            }
        }
        if (*(signed char *)(q + 0x20) >= 0) {
            if (*(short *)(q + 0xAA) == 0xBBA) {
                *(int *)(*(char **)(q + 0x68) + 0x258) = 1;
            }
        }
        if (*(short *)p < 0)
            break;
        p += 2;
    }
}

/* Sends the eight pairs of the level's angle table to the moby list: every pair
   of degrees becomes two angles, and the list keeps the moby's own handle. */
void func_L06_004576C8(char *a0)
{
    char *obj = *(char **)(a0 + 0x68);
    char *s0 = obj + 0x80;
    char *s1 = obj + 0xA0;
    float *p = (float *)D_L06_001B1890;
    float a1, a2;
    int i;

    for (i = 7; i >= 0; i--) {
        a1 = func_00283C28(p[0] * 0.017453294f);
        a2 = func_00283C28(p[1] * 0.017453294f);
        p += 2;
        func_002AF000(s1, 1.0f, a1, a2);
        s1 += 0x10;
        /* the third argument is the float 0.011489026f as a bit pattern */
        *(int *)s0 = func_L03_0042B6F8(D_L06_001A8F20, D_L06_001A8F40,
                                       0x3C3C3C78, a0, 25, 0);
        s0 += 4;
    }
}
