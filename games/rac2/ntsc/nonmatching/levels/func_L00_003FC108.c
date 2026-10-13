/* Best attempt, wave 6 w28. Last check.py line:
   func_L00_003FC108: 0 of 27 places match (src/levels/shared/L00_003F8E08.c)
   retail 284 bytes, produced 272; callee-saved mapping swapped s2/s3 (o vs a1)
   and the entry block order of the five stores differs. */

typedef int TI128 __attribute__((mode(TI)));

extern void *func_3100D8(int);
extern void func_2EE3D0(void *, void *, float);
extern void func_2EE108(void *, void *, void *);
extern float func_2EF240(int);
extern void func_3FC820(void *);

void *func_L00_003FC108(char *a0, char *a1, TI128 *a2, char *a3)
{
    char *o = func_3100D8(0xD5E);

    if (o != 0) {
        char *s0 = *(char **)(o + 0x68);
        float tmp[4];

        *(unsigned char *)(o + 0x20) = 0;
        *(unsigned char *)(o + 0x30) = 0xFF;
        *(unsigned char *)(o + 0x31) = 1;
        *(unsigned short *)(o + 0x32) = 0xFF;
        *(unsigned short *)(o + 0x34) |= 0x41;
        func_2EE3D0(s0 + 0x20, a1, 0.13333334f);
        *(TI128 *)(o + 0x10) = *a2;
        func_2EE3D0(tmp, s0 + 0x20, -1.5f);
        func_2EE108(s0 + 0x50, a0, tmp);
        *(int *)(s0 + 0x38) = 0x14;
        *(char **)(s0 + 0x34) = a3;
        *(float *)(s0 + 0x3C) = 1.0f / func_2EF240(0x14);
        func_3FC820(o + 0x10);
    }
    return o;
}
