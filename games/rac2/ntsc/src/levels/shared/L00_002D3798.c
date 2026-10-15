#include "include_asm.h"

#ifndef RAC2_T_V4_F6A70B950
#define RAC2_T_V4_F6A70B950
typedef int V4_F6a70b950 __attribute__((mode(TI)));
#endif

extern char D_00189E20_19[];

extern float func_00283180_6(float);

extern float func_00283198_6(float);

extern void func_00283410_4(float *m, char *src);

extern void func_002830C0_6(float *dst, float *src, float *m);

extern void func_00282CC0_16(V4_F6a70b950 *out, V4_F6a70b950 *in, float *v);

#ifndef RAC2_T_V4_FE23FE324
#define RAC2_T_V4_FE23FE324
typedef int V4_Fe23fe324 __attribute__((mode(TI)));
#endif

extern char D_00189E20_33[];

extern void func_00282C88_6();

extern void func_00283410_10();

extern void func_002830C0_11();

extern void func_00282CC0_37();

extern void func_00282F20_5(char *local, char *a, float k);

extern float func_00282DB0_2(char *a, char *src);

extern void func_00282D30_11(char *dst, char *src, float k);

extern void func_00282CF0_8(char *dst, char *src, char *local);

extern char D_00189E20_14[];

            /* 0x00189E20 */

#ifndef RAC2_T_V4_FC186F630
#define RAC2_T_V4_FC186F630
typedef int V4_Fc186f630 __attribute__((mode(TI)));
#endif

extern unsigned char D_00189E20_4[];

extern float func_00282E20_1(char *object);

extern void func_00283410_2(char *dst, char *src);

extern void func_00283698_1(char *dst, char *src);

extern void func_002830C0_1(char *dst, char *src, char *extra);

extern unsigned char D_00189E20_18[];

extern void func_00282F70(char *a, int b, float f);

extern void func_002830C0_5(char *a, char *b, char *c);

extern char D_00189E20_31[];

extern void func_002830C0_9(char *dst, char *src, char *extra);

#ifndef RAC2_T_V4_FD337D953
#define RAC2_T_V4_FD337D953
typedef int V4_Fd337d953 __attribute__((mode(TI)));
#endif

extern char D_00189E20_32[];

extern void func_002830C0_10(void *a, void *b, void *c);

#ifndef RAC2_T_PERSISTENT_F882F1178
#define RAC2_T_PERSISTENT_F882F1178
typedef struct {
    unsigned char pad0[4960];
    void *p1360;
    void *p1364;
    void *p1368;
    unsigned char pad1[4512];
    float f250C;
    float f2510;
    float f2514;
} Persistent_F882f1178;
#endif

#ifndef RAC2_T_NODE_F882F1178
#define RAC2_T_NODE_F882F1178
typedef struct {
    unsigned char pad[120];
    int field120;
} Node_F882f1178;
#endif

extern Persistent_F882f1178 D_00189E20_11;

extern int D_L00_001A8F00 __attribute__((sda));

extern float func_00283B98_4(float, float);

extern float func_00283198_3(float);

extern int func_00283D38_1(int, int, float);

extern void func_L00_00326048(int, float *, int, int, float);

/* Dispatches on one byte of a fixed global table: when it is zero it copies a 16-byte block from the second argument to the first and then accumulates two table-derived floats, each scaled by the third argument, into the first two words of that block; when it is 1 or 2 it builds a 16-byte vector from two table-derived values and a zero, fills two 64-byte matrices through helper routines, composes them into a vector and passes that vector with the two pointers to a closing routine. */
void func_L00_002D3798(V4_F6a70b950 *out, V4_F6a70b950 *in, float scale)
{
    char *p = D_00189E20_19;
    unsigned char mode;
    float z;

    mode = *(unsigned char *)(p + 8899);

    switch (mode) {
    case 0:
        *out = *in;
        *(float *)out += func_00283180_6(*(float *)(*(int *)(p + 8848) + 248)) * scale;
        *(float *)((char *)out + 4) += func_00283198_6(*(float *)(*(int *)(p + 8848) + 248)) * scale;
        break;
    case 1:
    case 2: {
        float m[16];
        float v[4];

        z = 0.0f;
        v[0] = func_00283180_6(z) * scale;
        v[1] = func_00283198_6(z) * scale;
        v[2] = z;
        func_00283410_4(m, (char *)*(int *)(p + 8848) + 240);
        func_002830C0_6(v, v, m);
        func_00282CC0_16(out, in, v);
        break;
    }
    }
}

/* Reads a byte state from a global root; when zero it copies a 16-byte vector and adds a float to its third element, otherwise (state 1 or 2) it builds a 4x4 matrix from a global object and transforms a vector by it. */
void func_L00_002D38A0(V4_Fe23fe324 *a0, V4_Fe23fe324 *a1, float f)
{
    char *p = D_00189E20_33;
    unsigned char raw = p[8899];
    int state = raw;
    float m[4][4];
    float v[4];

    if (state == 0) {
        *a0 = *a1;
        ((float *)a0)[2] += f;
        return;
    }
    if (state < 0)
        return;
    if (state >= 3)
        return;
    func_00282C88_6(v);
    v[2] = f;
    func_00283410_10(m, *(int *)(p + 8848) + 240);
    func_002830C0_11(v, v, m);
    func_00282CC0_37(a0, a1, v);
}

/* switches on a state byte in a shared table: copies 16 bytes and clears the third word, or computes a dot product that scales a vector and subtracts it into the destination */
void func_L00_002D3960(char *dst, char *src)
{
    char local[16];
    char *base;
    float f;

    base = D_00189E20_14;
    switch (*(unsigned char *)(base + 8899)) {
    case 0:
        *(V4_Fc186f630 *)dst = *(V4_Fc186f630 *)src;
        *(int *)(dst + 8) = 0;
        break;
    case 1:
        func_00282F20_5(local, base + 672, 1.0f);
        f = func_00282DB0_2(local, src);
        func_00282D30_11(local, local, f);
        func_00282CF0_8(dst, src, local);
        break;
    case 2:
        f = func_00282DB0_2(base + 704, src);
        func_00282D30_11(local, base + 704, f);
        func_00282CF0_8(dst, src, local);
        break;
    }
}

/* Reads one byte and one pointer from a shared global; returns a callee float when the byte is zero, returns 0.0f when the byte is outside 0..2, otherwise fills three stack locals through three callees and returns the last callee float. */
float func_L00_002D3A58(char *arg)
{
    char first[64];
    char second[64];
    char third[16];
    unsigned char *p = D_00189E20_4;
    int flag = p[8899];

    if (flag == 0)
        return func_00282E20_1(arg);
    if (flag < 0)
        return 0.0f;
    if (flag >= 3)
        return 0.0f;
    func_00283410_2(first, (char *)(*(int *)(p + 8848) + 240));
    func_00283698_1(second, first);
    func_002830C0_1(third, arg, second);
    return func_00282E20_1(third);
}

extern void func_00283098(void *a, void *b, void *c);

float func_L00_002D3B00(char *in)
{
    char *p = (char *)&D_00189E20_11;
    int mode = *(unsigned char *)(p + 8899);
    float local[4];

    if (mode == 0)
        return *(float *)(in + 8);
    if (mode < 0)
        return 0.0f;
    if (mode >= 3)
        return 0.0f;
    func_00283098(local, in, p + 64);
    return local[2];
}

extern void func_002830C0(void *a, void *b, void *c);

void func_L00_002D3B58(V4_F6a70b950 *dst, V4_F6a70b950 *src, float k)
{
    char *p = (char *)&D_00189E20_11;
    int mode = *(unsigned char *)(p + 8899);

    if (mode == 0) {
        *dst = *src;
        ((float *)dst)[2] = k;
        return;
    }
    if (mode < 0)
        return;
    if (mode >= 3)
        return;
    func_002830C0(dst, src, p + 64);
    ((float *)dst)[2] = k;
    func_002830C0(dst, dst, p);
}

/* reads a mode byte from a global block; when zero it forwards the arguments to one helper, otherwise it range-checks the byte and drives two helpers with the object and block pointers */
void func_L00_002D3BD8(char *a0, int a1, float f12)
{
    unsigned char *d = D_00189E20_18;
    float f20;
    int v;

    f20 = f12;
    v = d[8899];
    if (v == 0) {
        func_00282F70(a0, a1, f12);
        return;
    }
    if (v < 0)
        return;
    if (v >= 3)
        return;
    func_002830C0_5(a0, a1, (char *)d + 64);
    func_00282F70(a0, a0, f20);
    func_002830C0_5(a0, a0, (char *)d);
}

/* Flag-gated record update: copies 16 bytes src->dst and scales dst+8 by k when a byte flag is zero, otherwise runs two helper calls with DATA0+64 and DATA0 offsets and scales the same field between them. */
void func_L00_002D3C68(char *dst, char *src, float k)
{
    char *base = D_00189E20_31;
    int flag = *(unsigned char *)(base + 8899);

    if (flag == 0) {
        *(V4_Fd337d953 *)dst = *(V4_Fd337d953 *)src;
        *(float *)(dst + 8) = *(float *)(dst + 8) * k;
        return;
    }
    if (flag < 0)
        return;
    if (flag >= 3)
        return;
    func_002830C0_9(dst, src, base + 64);
    *(float *)(dst + 8) = *(float *)(dst + 8) * k;
    func_002830C0_9(dst, dst, base);
}

/* dispatches on one byte of a global table with a three-case switch: case 0 scales two floats of the destination by the incoming scale, cases 1 and 2 first run a helper over the destination with a table at +64, scale the same two floats and then run a second helper with the destination as both source and destination */
void func_L00_002D3CF8(float *dst, void *src, float scale)
{
    char *d = D_00189E20_32;
    int v = *(unsigned char *)(d + 8899);

    switch (v) {
    case 0:
        dst[0] = dst[0] * scale;
        dst[1] = dst[1] * scale;
        break;
    case 1:
    case 2:
        func_002830C0_10(dst, src, d + 64);
        dst[0] = dst[0] * scale;
        dst[1] = dst[1] * scale;
        func_002830C0_10(dst, dst, d);
        break;
    }

}

INCLUDE_ASM("asm/levels", func_L00_002D3D98);

INCLUDE_ASM("asm/levels", func_L00_002D3EA8);

#ifndef RAC2_T_ENTRY_262570
#define RAC2_T_ENTRY_262570
typedef struct {
    unsigned char pad0[142];
    unsigned short f8E;
    unsigned char rest[80];
} Entry_262570;
#endif

extern unsigned char D_L00_00139568[];

extern Entry_262570 D_L00_00262570[];

extern int D_L00_00139688[];

extern int D_0018B068;

int func_L00_002D3F30(int a0, int a1)
{
    int idx = a0;
    unsigned char b;
    Entry_262570 *e;
    int word;

    if (idx == -1)
        idx = D_0018B068;
    b = D_L00_00139568[idx];

    e = &D_L00_00262570[b];
    if (e->f8E == 0)
        return 1;
    word = D_L00_00139688[idx];
    if (word < a1)
        return 0;
    D_L00_00139688[idx] = word - a1;
    return 1;
}

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_002D3FB0(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_002D3FB8);

extern unsigned char D_L00_00139568[];

#ifndef RAC2_T_ENTRY_262570
#define RAC2_T_ENTRY_262570
typedef struct {
    unsigned char pad0[142];
    unsigned short f8E;
    unsigned char rest[80];
} Entry_262570;
#endif

extern Entry_262570 D_L00_00262570[];

extern int D_L00_00139688[];

extern int D_0018B068;

int func_L00_002D4040(int a0)
{
    int idx = a0;
    unsigned char b;
    Entry_262570 *e;

    if (idx == -1)
        idx = D_0018B068;
    b = D_L00_00139568[idx];

    e = &D_L00_00262570[b];
    if (e->f8E == 0)
        return 1;
    return D_L00_00139688[idx];
}

INCLUDE_ASM("asm/levels", func_L00_002D40A0);

extern int D_0018C0B4;

int func_L00_002D40F0(void)
{
    switch (D_0018C0B4) {
    case 98:
    case 99:
        return 1;
    case 100:
        return 2;
    }
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002D4138);

INCLUDE_ASM("asm/levels", func_L00_002D4140);

extern int func_003425F8(int, float, char *, char *);

void func_L00_002D49F0(void)
{
    char *p = (char *)&D_00189E20_11;

    if (func_003425F8(*(short *)(p + 0x24FE), *(float *)(p + 0x264), p + 0x80, p + 0x80)) {
        *(short *)(p + 0x216) = 4;
    }
}

extern int ref_L00_002D4A38_00 __attribute__((sda));

void func_L00_002D4A38(void)
{
    Node_F882f1178 *node;

    node = (Node_F882f1178 *)D_00189E20_11.p1360;
    if (node != 0) {
        if (ref_L00_002D4A38_00 == 2)
            D_00189E20_11.f250C = func_00283B98_4(D_00189E20_11.f250C, 0.020362177863717079f);
        else
            D_00189E20_11.f250C = func_00283B98_4(D_00189E20_11.f250C, 0.034906592220067978f);
        node->field120 = func_00283D38_1(0xd2d2d2, 0x285050,
                                 func_00283198_3(D_00189E20_11.f250C) * 0.5f + 0.5f);
    }
    node = (Node_F882f1178 *)D_00189E20_11.p1364;
    if (node != 0) {
        if (D_L00_001A8F00 != 2) {
            float limit;
            int which;

            limit = func_00283B98_4(D_00189E20_11.f2510, 0.049451004713773727f);
            D_00189E20_11.f2510 = limit;
            which = 0x1ee628;
            if (limit > 0.0f && limit < 1.9198623895645142f) {
                which = 0x1e1ed2;
            }
            node->field120 = func_00283D38_1(node->field120, which, 0.070000000298023224f);
        } else {
            D_00189E20_11.f2510 = func_00283B98_4(D_00189E20_11.f2510, 0.026179943233728409f);
            node->field120 = func_00283D38_1(0x1e1ed2, 0x1e1e50,
                                     func_00283198_3(D_00189E20_11.f2510) * 0.5f + 0.5f);
        }
    }
    if (D_00189E20_11.p1368 != 0) {
        if (D_L00_001A8F00 == 2)
            func_L00_00326048((int)D_00189E20_11.p1368, &D_00189E20_11.f2514, 0xdcdcdc, 0x323232,
                    0.026179943233728409f);
        else
            func_L00_00326048((int)D_00189E20_11.p1368, &D_00189E20_11.f2514, 0xdcdcdc, 0x323232,
                    0.040724355727434158f);
    }
}


INCLUDE_ASM("asm/levels", func_L00_002D4C28);

INCLUDE_ASM("asm/levels", func_L00_002D4D98);

INCLUDE_ASM("asm/levels", func_L00_002D4E58);

INCLUDE_ASM("asm/levels", func_L00_002D5000);

INCLUDE_ASM("asm/levels", func_L00_002D5098);

INCLUDE_ASM("asm/levels", func_L00_002D5158);

INCLUDE_ASM("asm/levels", func_L00_002D5940);

INCLUDE_ASM("asm/levels", func_L00_002D5E30);

INCLUDE_ASM("asm/levels", func_L00_002D5EB8);

INCLUDE_ASM("asm/levels", func_L00_002D6418);
