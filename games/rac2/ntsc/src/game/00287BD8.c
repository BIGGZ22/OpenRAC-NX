#include "include_asm.h"

/* Family 8c659bbcffcd2ad3 -- 324 bytes, 27 level placements. */

#ifndef RAC2_T_BLOCK48_F8C659BBC
#define RAC2_T_BLOCK48_F8C659BBC
typedef struct { char pad[48]; } Block48_F8c659bbc;
#endif

#ifndef RAC2_T_BLOCK16_F8C659BBC
#define RAC2_T_BLOCK16_F8C659BBC
typedef struct { char pad[16]; } Block16_F8c659bbc;
#endif

extern char D_L00_001B1B10[] __attribute__((sda));

extern char D_L00_001B8DC0[];

extern char D_L00_001B8FF0[];

extern char D_L00_001A8E30 __attribute__((sda));

extern void func_00282CF0(char *a, char *b, char *c);

extern void func_00282F20(char *a, char *b, float f);

extern void func_002833F0(char *a, char *b);

extern void func_00283738(char *a, char *b, char *c);

extern void func_00282DD8(char *a, char *b, char *c);

extern float func_00282DF0(char *a);

extern void func_002ACE78(char *a, char *b, float f);

extern void func_002AAC80(char *a, float f1, float f2, float f3, float f4);

extern void func_00283958(char *a, char *b);

void func_00287BD8(char *arg0, int arg1)
{
    Block48_F8c659bbc a;
    Block48_F8c659bbc b;
    Block48_F8c659bbc c;
    Block16_F8c659bbc d;
    Block16_F8c659bbc e;
    float v;

    func_00282CF0((char *)&e, D_L00_001B8DC0, D_L00_001B1B10);
    func_00282F20((char *)&e, (char *)&e, 1.0f);
    func_002833F0((char *)&c, (char *)&D_L00_001A8E30);
    func_00283738((char *)&b, D_L00_001B8DC0 + 560, (char *)&c);
    func_00282DD8((char *)&d, (char *)&b, (char *)&e);
    v = func_00282DF0((char *)&d) * 0.5f;
    if (arg1 != 0) {
        func_002ACE78((char *)&d, (char *)&d, -v);
    } else {
        func_002AAC80(arg0, v, 5.8177649043500423431396e-04f,
                5.8177649043500423431396e-04f,
                5.2359886467456817626953e-02f);
        func_002ACE78((char *)&d, (char *)&d, -*(float *)arg0);
    }
    func_00283958((char *)&d, (char *)&a);
    func_00283738(D_L00_001B8FF0, (char *)&a, D_L00_001B8FF0);
}

extern float D_L00_001A8DD0 __attribute__((sda));
void func_00287D20(float *arg0) { char temp[0x40]; float value = *arg0 * 0.0002908882452175021f * D_L00_001A8DD0; func_002ACE78(temp, D_L00_001B8FF0, value); func_00283958(temp, temp + 0x10); func_00283738(D_L00_001B8FF0, temp + 0x10, D_L00_001B8FF0); }

extern void func_00282CC0(char *a, char *b, char *c);

extern int D_L00_001A8DC8 __attribute__((sda));

void func_00287D98(char *arg0)
{
    func_00282CF0(D_L00_001B8DC0, D_L00_001B8DC0, D_L00_001B1B10);
    func_00282CC0(D_L00_001B8DC0, D_L00_001B8DC0, arg0);
    func_00282F20(D_L00_001B8DC0, D_L00_001B8DC0, (float)D_L00_001A8DC8);
    func_00282CC0(D_L00_001B8DC0, D_L00_001B8DC0, D_L00_001B1B10);
}
