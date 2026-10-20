/* 0 of 27 places match - last line: 01_oozla: 324 bytes, expected 416

   The calls and the record writes are in place, but this cc1 does not keep the six
   float locals the way retail does.  Retail holds the three parameters and the three
   computed values in $f20-$f25 and saves all six in the frame (0x60..0x88 of a 0x90
   frame); this cc1 keeps one of them in $f20, rematerialises the constants
   (1.0f, 0.5f) and drops the frame to 0x70, so the body comes out 23 instructions
   short.

   The float constants are read from the retail bytes: 0x4788B800 = 70000.0f,
   0x48127C00 = 150000.0f, 0x3E4CCCCD = 0.2f, 0x3F800000 = 1.0f, 0x3F000000 = 0.5f.
   The six calls are func_002A7878 (twice), func_00283D38, func_002A7A00,
   func_00282CC0 and func_002BDCE8; the two delay slots that carry the float
   results are the ones that decide where each result is used. */
#include "common.h"

extern float func_002A7878(float, float);

extern int func_00283D38(int, int, float);

extern void func_002A7A00(char *, float);

extern void func_00282CC0(char *, char *, char *);

extern int func_002BDCE8(int, int, char *, int, int);

void func_L01_002E1680(int a0, int a1, int a2, int a3, int t0, float f12, float f13, float f14)
{
    float buf[4];
    int s1 = a2;
    float f20 = 0.0f;
    float f21 = f13;
    float f22 = 1.0f;
    float f23 = f14;
    float f24 = f12;
    float f25 = 0.5f;
    int s0;
    char *p;

    if (a2 <= 0)
        return;

    do {
        f20 = f21 * func_002A7878(70000.0f, 150000.0f);
        s0 = func_00283D38(0, a3, func_002A7878(0.2f, 1.0f));
        func_002A7A00((char *)buf, f24);
        func_00282CC0((char *)buf, (char *)buf, (char *)a1);
        p = (char *)func_002BDCE8(a0, 0, (char *)buf, a3, 0);
        if (p != 0) {
            *(unsigned short *)(p + 0xA) = 0x78;
            *(int *)(p + 0x24) = 2;
            *(unsigned char *)(p + 0x2A) = (unsigned char)(s0 >> 24);
            *(unsigned char *)(p + 0x2B) = (unsigned char)p[0xA];
            if (t0 != 0)
                p[3] = 0x44;
        }
        s1--;
    } while (s1 != 0);
}
