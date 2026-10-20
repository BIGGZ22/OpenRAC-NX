#include "include_asm.h"

struct vec2 {
    float x;
    float y;
};

struct quad {
    int v[4];
};

struct row224 {
    unsigned char pad[6];
    short id;
    unsigned char rest[0xD8];
};

extern int D_001ADB10 __attribute__((sda));
/* D_001A7A18 is read as element 4 of the 8-byte object at D_001A7A10: an index past a small-data object
   makes cc1 emit the two-word absolute load (not delay-slot material) that retail has, where a plain short
   comes out $gp-relative in the call's delay slot */
extern short D_001A7A10[4];
extern unsigned char D_001A7AF8[];
extern unsigned char D_00139568[];
extern struct row224 D_00239BA0[];
/* incomplete arrays and a 16-byte struct are not small data: D_00138344 stays a lui/lw pair on two registers and
   the copy sources D_001AE270/D_001AE2A8 are reached through a lui/addiu address, as in retail */
extern int D_00138344[];
extern struct quad D_001AE270;
extern struct vec2 D_001AE2A8[];
extern float D_001AE280 __attribute__((sda));
extern float D_001AE284;
extern float D_001AE288 __attribute__((sda));
extern float D_001AE28C;
extern float D_001AE290 __attribute__((sda));
extern float D_001AE294;
extern float D_001AE298 __attribute__((sda));
extern float D_001AE29C;
extern float D_001AE2A0 __attribute__((sda));
extern float D_001AE2A4;
extern float D_001AE2B0 __attribute__((sda));
extern float D_001AE2B4;
extern float D_001AE2B8 __attribute__((sda));
extern float D_001AE2BC;
extern void func_00287FF8(int, int);
extern void func_00343EE8(char *, unsigned int);
extern void func_00344298(char *, unsigned int);
extern void func_00343FF8(char *, unsigned int);
extern void func_00344160(char *, unsigned int);
extern void func_00335E38(char *, float, float, float, float);
extern int func_002A9588(int, int, int, int, int);
extern int *func_00335E20(char *);
extern void func_00344848(char *);

/* menu update: clears the index at +0x444, dispatches on the mode at +0x448 (0 to 3), stores the id of the
   selected entry in D_001ADB10, then repositions ten widgets from the position pointer at +0x430; returns 0 */
int func_00344478(char *self, unsigned int buttons)
{
    *(int *)(self + 0x444) = 0;
    D_001ADB10 = -1;
    func_00287FF8(6, D_001A7A10[4]);
    switch (*(int *)(self + 0x448)) {
    case 0:
        func_00343EE8(self, buttons);
        break;
    case 1:
        func_00344298(self, buttons);
        break;
    case 2:
        func_00343FF8(self, buttons);
        break;
    case 3:
        func_00344160(self, buttons);
        break;
    }
    if (*(int *)(self + 0x448) == 3) {
        struct quad table = D_001AE270;
        int idx = *(int *)(self + 0x444);

        D_001ADB10 = table.v[idx];
    } else {
        int idx = *(int *)(self + 0x444);

        if (D_001A7AF8[idx])
            D_001ADB10 = (D_00239BA0 + D_00139568[idx])->id;
    }
    {
        float zero = 0.0f;
        struct vec2 offset = D_001AE2A8[0];
        struct vec2 **pos = (struct vec2 **)(self + 0x430);

        func_00335E38(self, D_001AE280 + (*pos)->x, D_001AE284 + (*pos)->y, zero, zero);
        func_00335E38(self + 0x4C, D_001AE2A0 + (*pos)->x, D_001AE2A4 + (*pos)->y, zero, zero);
        func_00335E38(self + 0x98, D_001AE288 + (*pos)->x, D_001AE28C + (*pos)->y, zero, zero);
        func_00335E38(self + 0xE4, D_001AE290 + (*pos)->x, D_001AE294 + (*pos)->y, zero, zero);
        func_00335E38(self + 0x130, D_001AE298 + (*pos)->x, D_001AE29C + (*pos)->y, zero, zero);
        func_00335E38(self + 0x1C8, offset.x + (*pos)->x, offset.y + (*pos)->y, zero, zero);
        func_00335E38(self + 0x298, D_001AE2B0 + (*pos)->x, D_001AE2B4 + (*pos)->y, zero, zero);
        func_00335E38(self + 0x210, D_001AE2B8 + (*pos)->x, D_001AE2BC + (*pos)->y, zero, zero);
        func_00335E38(self + 0x130, D_001AE2B8 + (*pos)->x, D_001AE2BC + (*pos)->y, zero, zero);
        func_00335E38(self + 0x17C, D_001AE2B8 + (*pos)->x, D_001AE2BC + (*pos)->y, zero, zero);
    }
    if (D_00138344[0] & 0xF000)
        func_002A9588(0, 0, 1, 0, 1);
    *func_00335E20(self + 0x17C) = func_002A9588(0x60442D00, 0x70FFFEED, 20, 0, 0);
    func_00344848(self);
    return 0;
}
