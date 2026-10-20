#include "include_asm.h"

/* Measured partial C views; original types and object ownership are unknown. */
typedef struct {
    unsigned char prefix[0x188];
    unsigned char wrapped;
    unsigned char value;
} QwenRecovery_538934d89776_WrapperCollectState132;

extern unsigned char ref_L00_002D6FD8_01[3];

extern int ref_L00_002D6FD8_00;

extern QwenRecovery_538934d89776_WrapperCollectState132 ref_L00_002D6FD8_03;

extern int ref_L00_002D6FD8_02(int words[2]);

void func_L00_002D6FD8(void)
{
    int words[4];
    ref_L00_002D6FD8_02(words);
    if (ref_L00_002D6FD8_01[2] != 0) {
        ref_L00_002D6FD8_00 += words[0];
        if (ref_L00_002D6FD8_00 >= 256) {
            ref_L00_002D6FD8_03.wrapped = 1;
            ref_L00_002D6FD8_00 -= 255;
        } else {
            ref_L00_002D6FD8_03.wrapped = 0;
        }
        ref_L00_002D6FD8_03.value = ((unsigned char *)words)[4];
    } else {
        words[0] = 0;
        words[1] = 0;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002D7060);

/* Clears one byte of each of the 32 64-byte records that end at the level pool root plus
   0x7DD, walking down one record at a time; the stored -1 is the function's return value. */
extern unsigned char D_L00_001BB9F0[];
extern int D_L00_001B1884;

int func_L00_002D7068(void)
{
    unsigned char *p = D_L00_001BB9F0;
    int i;
    int r = -1;

    p += 0x7DD;
    for (i = 31; i >= 0; i--) {
        *p = 0;
        p -= 64;
    }
    D_L00_001B1884 = r;
    return r;
}

/* Partial 64-byte pool record, measured from the retail loads and stores. */
typedef struct {
    float f0;
    float f4;
    float f8;
    float f12;
    float f16;
    float f20;
    unsigned char f24;
    unsigned char f25;
    short f26;
    unsigned char f28;
    unsigned char f29;
    unsigned char f30;
    unsigned char f31;
    unsigned char pad32[16];
    short f48;
    short f50;
    int f52;
    int f56;
    int f60;
} PoolRecord64;

extern PoolRecord64 D_L16_001BC730[];

PoolRecord64 *func_L00_002D70A0(int a)
{
    PoolRecord64 *p = D_L16_001BC730;
    int i;

    for (i = 0; i < 32; i++) {
        if (p->f29 == 0) {
            p->f0 = -1.0f;
            p->f4 = -1.0f;
            p->f8 = -1.0f;
            p->f12 = -1.0f;
            p->f16 = -1.0f;
            p->f20 = -1.0f;
            p->f28 = a;
            p->f52 = 6;
            p->f50 = 120;
            p->f29 = 1;
            p->f56 = -1;
            p->f24 = 0;
            p->f25 = 0;
            p->f26 = 0;
            p->f30 = 0;
            p->f31 = 0;
            p->f60 = 0;
            p->f48 = 0;
            return p;
        }
        p++;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002D7138);
