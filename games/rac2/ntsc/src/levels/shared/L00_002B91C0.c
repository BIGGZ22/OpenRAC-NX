#include "include_asm.h"

typedef unsigned char u8;

#ifndef RAC2_T_TI_F41EB487E
#define RAC2_T_TI_F41EB487E
typedef int TI_F41eb487e __attribute__((mode(TI)));
#endif

extern void func_002830C0_2(void *a, void *b, void *c);

extern void func_00282D30_4(void *a, void *b, float value);

extern void func_L00_002B8F00(void *a, void *b, int c, float p, float q, float r);

extern void func_00282CF0_3(void *a, void *b, void *c);

extern void func_00282CC0_3(void *a, void *b, void *c);

#ifndef RAC2_T_VEC4_F41EB487E
#define RAC2_T_VEC4_F41EB487E
typedef struct {
    int a;
    int b;
    float c;
    float d;
} Vec4_F41eb487e;
#endif

#ifndef RAC2_T_RESIDENTSTATE_F41EB487E
#define RAC2_T_RESIDENTSTATE_F41EB487E
typedef struct {
    u8 pad000[0x80];
    u8 area080[0x218 - 0x80];
    short h218;
    u8 pad21a[0x2a0 - 0x21a];
    TI_F41eb487e q2A0;
    u8 pad2b0[0x2c0 - 0x2b0];
    u8 area2C0[0x31c - 0x2c0];
    float f31C;
    u8 pad320[0x34e - 0x320];
    short h34E;
    u8 pad350[0x22c3 - 0x350];
    u8 b22C3;
} ResidentState_F41eb487e;
#endif

extern ResidentState_F41eb487e D_00189E20_5;

extern char D_00189F80[];

extern float func_L00_002D3A58_1(void *p);

extern void func_L00_002D3B58_1(void *a, void *b, float f);

extern void func_L00_002D3BD8_2(void *a, void *b, float f);

extern void func_00282CC0_28(void *a, void *b, void *c);

extern void func_00282C88_5(void *a);

extern void  func_002AF000_1(char *p, float a, float b, float c);

extern void  func_00282CC0_18(char *p, char *q, char *r);

extern int   func_002761F8_1(void *a, void *b, int c, int d, int e);

extern void  func_L00_002D36E0_1(void *p, int n, float a, float b, float c);

extern void  func_00282CF0_8(void *dst, void *src, void *arg);

extern float func_00283248_5(float a, float b);

extern float func_00282E20_3(void *p);

extern char  D_L00_001B8C80_2[];

extern char  D_L00_001BF220[];

extern char  D_00189E20_21[];

/* initialises three 16-byte records, folds a resident 32-byte quad into the first of them, then for the stored mode 2 runs a four-argument fallback and for mode 1 with a short or low field re-copies the resident quad; finally scales and copies the two remaining records through three more calls */
void func_L00_002B91C0(void)
{
    u8 scratch[64];
    Vec4_F41eb487e A, B, C;
    float hi = 0.035f;
    float lo = 0.3f;

    *(TI_F41eb487e *)&A = 0;
    *(TI_F41eb487e *)&B = 0;
    *(TI_F41eb487e *)&C = 0;

    A.c = 1.0f;
    B.c = 0.8f;
    C.c = 0.8f;

    func_002830C0_2(&B, &B, &D_00189E20_5);

    *(TI_F41eb487e *)&A = *(TI_F41eb487e *)&D_00189E20_5.q2A0;
    if (D_00189E20_5.b22C3 == 2) {
        func_00282D30_4(&A, &D_00189E20_5.area2C0, -1.0f);
    }

    if (D_00189E20_5.b22C3 == 1 && (D_00189E20_5.h34E < 10 || D_00189E20_5.f31C < 1.0f))
        *(TI_F41eb487e *)&A = *(TI_F41eb487e *)&D_00189E20_5.q2A0;

    if (D_00189E20_5.h34E != 0 && D_00189E20_5.h218 != 0) {
        lo = 0.001f;
        hi = lo;
    }

    func_L00_002B8F00(&A, scratch, 0, hi, lo, 0.0f);

    if (D_00189E20_5.b22C3 == 1 && D_00189E20_5.h34E != 0) {
        func_002830C0_2(&C, &C, scratch);
        func_00282CF0_3(&D_00189E20_5.area080, &D_00189E20_5.area080, &C);
        func_00282CC0_3(&D_00189E20_5.area080, &D_00189E20_5.area080, &B);
    }
}

INCLUDE_ASM("asm/levels", func_L00_002B9380);

/* reads a scalar through DATA0 and, when it exceeds 0.001f, re-normalises the vector at DATA0 (and DATA0-112) from the elapsed float, clamping the amount at zero; otherwise it runs the plain refresh entry point. */
void func_L00_002B94D8(float a0)
{
    char *p = D_00189F80;
    float x = a0;
    float r = func_L00_002D3A58_1(p);
    float d;

    if (0.001f < r) {
        func_L00_002D3B58_1(p, p, 0.0f);
        d = r - x;
        if (d < 0.0f)
            d = 0.0f;
        func_L00_002D3BD8_2(p, p, d);
        func_00282CC0_28(p - 112, p - 112, p);
    } else {
        func_00282C88_5(p);
    }
}

INCLUDE_ASM("asm/levels", func_L00_002B9598);

/* The resident block the slot table lives in, and the two vector helpers. */
extern char D_00189E20_2[];

extern char D_00189E60[];

extern void func_2EE5D8(void *a0, void *a1, void *a2);

extern float func_2EE7A8(float a, float b);

extern void func_31DCF0(void *a0, float f12, float f13);

/* Reads two floats of the resident block through a helper, folds them with a second helper,
   and passes the result with the second argument to the third. */
void func_L00_002B9B38(void *a0, void *a1)
{
    float v[2];

    func_2EE5D8(v, a0, D_00189E60);
    func_31DCF0(a1, func_2EE7A8(v[0], v[1]), 0.0f);
}

/* The resident id this forwards, and the helper it calls. */
extern int D_0018C0B0;

extern void func_35F3A8(void *a0, void *a1, int a2);

/* Forwards both of its arguments and the resident id to the helper. */
void func_L00_002B9B88(void *a0, void *a1)
{
    func_35F3A8(a0, a1, D_0018C0B0);
}

INCLUDE_ASM("asm/levels", func_L00_002B9BB0);

/* Claims the first free slot of the eight 8-byte entries at resident +0x23EC: a slot whose
   leading short is clear takes the three arguments, a pending marker and its two extra
   fields, and the search stops at the first one taken. */
void func_L00_002B9C58(int a0, int a1, int a2)
{
    char *base = D_00189E20_2;
    short *slot = (short *)(base + 0x23EC);
    int i;

    for (i = 0; i < 8; i++, slot += 4) {
        if (slot[0] != 0)
            continue;
        slot[0] = 1;
        slot[3] = a2;
        slot[2] = a1;
        slot[1] = a0;
        return;
    }
}

INCLUDE_ASM("asm/levels", func_L00_002B9CA8);

INCLUDE_ASM("asm/levels", func_L00_002B9D60);

INCLUDE_ASM("asm/levels", func_L00_002B9F00);

INCLUDE_ASM("asm/levels", func_L00_002BA138);

INCLUDE_ASM("asm/levels", func_L00_002BA220);

/* queries a resident table with a stack request and blends a vector back into the resident record */
int func_L00_002BA410(void)
{
    char buf[48];
    char *base = D_L00_001B8C80_2;
    char *res;
    float v;

    func_002AF000_1(buf, 10.0f, *(float *)(base + 344), -*(float *)(base + 340));
    func_00282CC0_18(buf, buf, base + 320);
    res = D_00189E20_21;
    if (func_002761F8_1(base + 320, buf, 4, *(int *)(res + 8848), 0)) {
        func_L00_002D36E0_1(buf + 16, 1, 0.5f, 0.0f, 0.5f);
        func_00282CF0_8(buf + 32, D_L00_001BF220, buf + 16);
        v = func_00283248_5(*(float *)(buf + 32), *(float *)(buf + 36));
        *(float *)(res + 3076) = v;
        v = func_00282E20_3(buf + 32);
        *(float *)(res + 3080) = func_00283248_5(v, *(float *)(buf + 40));
        return 1;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002BA508);
