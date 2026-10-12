#include "include_asm.h"

extern void func_L00_002AED68(void);

extern void func_L00_002AF860(void);

extern void func_L00_002AEB18(void);

extern float func_00282C48_2(float d);

#ifndef RAC2_T_ROW_F3392E4BC
#define RAC2_T_ROW_F3392E4BC
typedef float Row_F3392e4bc[4];
#endif

extern char D_00189E20_12[];

extern void func_0029F338_1(char *entry);

#ifndef RAC2_T_V4_FA012328C
#define RAC2_T_V4_FA012328C
typedef int V4_Fa012328c __attribute__((mode(TI)));
#endif

extern char D_00189F20[];

extern char D_00189F30[] __attribute__((sda));

extern void func_L00_002B39D8(void);

extern void func_00282C88_4(void *p);

extern void func_002B8518(void);

extern void func_L00_002D3CF8_1(int, int);

extern void func_L00_002D3C68_1(int, int, float);

extern void func_L00_002D3BD8_1(int, int);

extern void func_L00_002D3B58(int, int, float);

typedef unsigned char u8;

typedef int s32;

typedef float f32;

extern u8 D_00189E20_1[];

typedef struct { u8 prefix[0xc38]; f32 plane; f32 depth; } GornFloatInterval64;

#ifndef RAC2_T_V4_F6B9E759F
#define RAC2_T_V4_F6B9E759F
typedef struct {
    f32 x, y, z, w;
} V4_F6b9e759f;
#endif

#ifndef RAC2_T_M4_F6B9E759F
#define RAC2_T_M4_F6B9E759F
typedef struct {
    V4_F6b9e759f r[4];
} M4_F6b9e759f;
#endif

extern V4_F6b9e759f D_00189EA0_1[];

extern void func_00283410_5(M4_F6b9e759f *, V4_F6b9e759f *);

extern void func_00283788_3(M4_F6b9e759f *, M4_F6b9e759f *, V4_F6b9e759f *);

extern void func_002AB6D8_3(M4_F6b9e759f *, V4_F6b9e759f *);

extern int func_002830C0(float *, char *, char *);

extern float func_00283248_1(float, float);

extern float func_002AA898(float *, char *, int, float, float, float, float);

extern void func_L00_002B3DC0_1(float, float, float);

extern char D_00189FB0[];

#ifndef RAC2_T_V4S_F31E44309
#define RAC2_T_V4S_F31E44309
typedef struct {
    f32 x, y, z, w;
} V4S_F31e44309;
#endif

extern V4S_F31e44309 D_00189E20_10[];

extern void func_00282D30_6(V4S_F31e44309 *, V4S_F31e44309 *, f32);

/* calls three measured no-argument functions in sequence */
void func_L00_002AFBC0(void) {
 func_L00_002AED68();
 func_L00_002AF860();
 func_L00_002AEB18();
}

INCLUDE_ASM("asm/levels", func_L00_002AFBF0);

/* Nested 3x3 sweep over two rows-of-four arrays: sums the results of calling the same routine with the difference of each of the nine element pairs, returns the sum. */
float func_L00_002B0480(Row_F3392e4bc *a, Row_F3392e4bc *b)
{
    float sum = 0.0f;
    int i, j;

    for (i = 0; i < 3; i++)
        for (j = 0; j < 3; j++)
            sum += func_00282C48_2(a[i][j] - b[i][j]);
    return sum;
}

INCLUDE_ASM("asm/levels", func_L00_002B0528);

INCLUDE_ASM("asm/levels", func_L00_002B0968);

/* for one fixed 80-byte slot: stores a value, rewrites a state word, dispatches through a function pointer at entry+100 unless the entry is null or carries a 253/254 sentinel, then releases two stored entry pointers, skipping the second when the slot index is 3 */
void func_L00_002B1230(int slot, int value)
{
    char *e;
    void (*fn)(char *);

    {
        char *p = D_00189E20_12 + slot * 80;

        *(int *)(p + 4660) = value;
        if (*(int *)(p + 4676) != 3) {
            *(int *)(p + 4676) = 3;
            e = *(char **)(p + 4640);
            if (e != 0) {
                if (*(unsigned char *)(e + 32) != 254) {
                    if (*(unsigned char *)(e + 32) != 253) {
                        fn = *(void (**)(char *))(e + 100);
                        if (fn != 0)
                            fn(e);
                    }
                }
            }
        }
    }
    {
        char *q = D_00189E20_12 + slot * 80;

        e = *(char **)(q + 4640);
        *(int *)(q + 4676) = 0;
        *(int *)(q + 4680) = 0;
        if (e != 0) {
            func_0029F338_1(e);
            *(char **)(q + 4640) = 0;
        }
        e = *(char **)(q + 4644);
        if (e != 0 && slot != 3) {
            func_0029F338_1(e);
            *(char **)(q + 4644) = 0;
        }
    }
}

INCLUDE_ASM("asm/levels", func_L00_002B1308);

INCLUDE_ASM("asm/levels", func_L00_002B1350);

INCLUDE_ASM("asm/levels", func_L00_002B1F90);

INCLUDE_ASM("asm/levels", func_L00_002B2120);

INCLUDE_ASM("asm/levels", func_L00_002B2200);

INCLUDE_ASM("asm/levels", func_L00_002B2318);

INCLUDE_ASM("asm/levels", func_L00_002B2A40);

/* Zeroes a 16-byte global block, calls a setup helper, runs a per-slot reset helper over seven 16-byte units of a global table, then clears three word fields of a block 256 bytes below the table base. */
void func_L00_002B2BD0(void)
{
    char *p;

    *(volatile V4_Fa012328c *)D_00189F30 = 0;
    func_L00_002B39D8();
    p = D_00189F20;
    func_00282C88_4(p);
    func_00282C88_4(p - 16);
    func_00282C88_4(p + 16);
    func_00282C88_4(p + 32);
    func_00282C88_4(p + 48);
    func_00282C88_4(p + 96);
    func_00282C88_4(p - 96);
    p = p - 256;
    *(int *)(p + 420) = 0;
    *(int *)(p + 376) = 0;
    *(int *)(p + 368) = 0;
    func_002B8518();
}

INCLUDE_ASM("asm/levels", func_L00_002B2C50);

/* dispatch on the second integer argument: each arm runs a pair of sub-calls passing the first argument twice, the second call also forwarding the float argument */
void func_L00_002B2C58(int a, int b, float x, float y)
{
    if (b != 0) {
        func_L00_002D3CF8_1(a, a);
        func_L00_002D3C68_1(a, a, y);
    } else {
        func_L00_002D3BD8_1(a, a);
        func_L00_002D3B58(a, a, y);
    }
}

INCLUDE_ASM("asm/levels", func_L00_002B2CC8);

INCLUDE_ASM("asm/levels", func_L00_002B30E8);

/* tests the signed short identifier at offset 0xaa of a nonnull object against 71 */
s32 func_L00_002B3248(u8 *object) {
 if(object==0) return 0;
 return *(short *)(object+0xaa)==71;
}

INCLUDE_ASM("asm/levels", func_L00_002B3268);

INCLUDE_ASM("asm/levels", func_L00_002B32F8);

INCLUDE_ASM("asm/levels", func_L00_002B35A0);

INCLUDE_ASM("asm/levels", func_L00_002B36E0);

INCLUDE_ASM("asm/levels", func_L00_002B3900);

/* Test whether a scalar lies below a plane within its configured depth, rejecting unordered values */
int func_L00_002B3998(f32 value)
{
    GornFloatInterval64 *root = (GornFloatInterval64 *)D_00189E20_1;
    if (value < root->plane) {
        if (root->plane - value <= root->depth)
            return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002B39D8);

INCLUDE_ASM("asm/levels", func_L00_002B39F0);

INCLUDE_ASM("asm/levels", func_L00_002B3D10);

/* builds one 64-byte block from a 3-float vector of the incoming scalars and another from a static table record one element in, combines them, then scales the result again from that record */
void func_L00_002B3DC0(f32 a1, f32 a2, f32 a3) {
    V4_F6b9e759f in;
    M4_F6b9e759f m1;
    M4_F6b9e759f m2;
    in.x = a1;
    in.y = a2;
    in.z = a3;
    func_00283410_5(&m1, &in);
    func_00283410_5(&m2, &D_00189EA0_1[1]);
    func_00283788_3(&m2, &m2, &m1);
    func_002AB6D8_3(&m2, &D_00189EA0_1[1]);
}

/* Fills a two-float scratch buffer from one region of a fixed global, passes it with three forwarded floats and an out-parameter to another callee, stores the returned float back into the global, then feeds that out value to a final callee. */
void func_L00_002B3E40(int a0, float a1, float a2, float a3)
{
    float tmp[2];
    char *p = D_00189FB0;
    float out;
    float r;

    func_002830C0(tmp, p, p - 336);
    r = func_00283248_1(tmp[0], tmp[1]);
    out = 0.0f;
    *(float *)(p + 24) = func_002AA898(&out, p + 20, a0, r, a1, a2, a3);
    func_L00_002B3DC0_1(0.0f, 0.0f, out);
}

INCLUDE_ASM("asm/levels", func_L00_002B3EF8);

INCLUDE_ASM("asm/levels", func_L00_002B3F60);

INCLUDE_ASM("asm/levels", func_L00_002B3F68);

INCLUDE_ASM("asm/levels", func_L00_002B41B8);

INCLUDE_ASM("asm/levels", func_L00_002B4208);

INCLUDE_ASM("asm/levels", func_L00_002B42D8);

INCLUDE_ASM("asm/levels", func_L00_002B4308);

INCLUDE_ASM("asm/levels", func_L00_002B4458);

INCLUDE_ASM("asm/levels", func_L00_002B47A8);

INCLUDE_ASM("asm/levels", func_L00_002B4900);

INCLUDE_ASM("asm/levels", func_L00_002B5250);

INCLUDE_ASM("asm/levels", func_L00_002B54F8);

INCLUDE_ASM("asm/levels", func_L00_002B5590);

INCLUDE_ASM("asm/levels", func_L00_002B55F0);

INCLUDE_ASM("asm/levels", func_L00_002B5A88);

INCLUDE_ASM("asm/levels", func_L00_002B5B60);

/* scales four 16-byte records of a static table in place by a scalar, then multiplies one float field of a later record by the same scalar */
void func_L00_002B6118(f32 a0) {
    V4S_F31e44309 *p = &D_00189E20_10[17];
    func_00282D30_6(p, p, a0);
    func_00282D30_6(p + 1, p + 1, a0);
    func_00282D30_6(p - 2, p - 2, a0);
    func_00282D30_6(p + 5, p + 5, a0);
    D_00189E20_10[27].y = D_00189E20_10[27].y * a0;
}

INCLUDE_ASM("asm/levels", func_L00_002B6198);

INCLUDE_ASM("asm/levels", func_L00_002B6560);

INCLUDE_ASM("asm/levels", func_L00_002B6BC8);

INCLUDE_ASM("asm/levels", func_L00_002B75A0);

INCLUDE_ASM("asm/levels", func_L00_002B7718);
