#include "include_asm.h"

extern float func_00283B98_2(float, float);

extern float func_00283180(float);

extern float func_00283198(float);

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_15(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

extern int func_002A7820_2(int, int);

extern float func_002A7878_5(float, float);

extern float func_002A7940_2(void);

extern void func_00281EB8(char *, int, int, int, float, float, float);

extern void func_002A3F10(char *p, int one, int *w, char *out);

extern void func_00282D30_3(char *a, char *out, float f);

extern void func_00282CC0_4(char *a, char *b, char *c);

extern void func_00283098_2(char *a, char *b, char *c);

extern char D_L00_001A8F20[] __attribute__((sda));

extern void func_0029FCC0_1(int a, int b, void *c);

extern int func_L00_003AB9C0(int a, void *b, int c, char *d, float e, float f, float g, float h);

extern void func_00282F20_1(char *, char *, float);

extern void func_00282DD8(char *, char *, char *);

extern char *func_L00_0032A088_1();

extern int *func_L00_0032A0B8_1();

INCLUDE_ASM("asm/levels", func_L00_003A0F30);

INCLUDE_ASM("asm/levels", func_L00_003A1580);

INCLUDE_ASM("asm/levels", func_L00_003A1F68);

INCLUDE_ASM("asm/levels", func_L00_003A2018);

/* Shared native body; original game role is unknown. */
void func_L00_003A2138(int unused, float *p, int n)
{
    float *v;
    int count;
    if (n <= 0)
        return;
    v = p;
    count = n;
    do {
        v[0] = func_00283B98_2(v[0], v[3]);
        v[1] = func_00283B98_2(v[1], v[4]);
        v[2] = func_00283B98_2(v[2], v[5]);
        v[9] = func_00283B98_2(v[9], v[12]);
        v[10] = func_00283B98_2(v[10], v[13]);
        v[11] = func_00283B98_2(v[11], v[14]);
        v[20] = func_00283180(v[0]) * v[6];
        v[21] = func_00283198(v[1]) * v[7];
        v[22] = -func_00283198(v[2]) * v[8];
        v[24] = func_00283180(v[9]) * v[15];
        v[25] = func_00283198(v[10]) * v[16];
        v[26] = -func_00283198(v[11]) * v[17];
        v += 32;
    } while (--count != 0);
}

INCLUDE_ASM("asm/levels", func_L00_003A2258);

INCLUDE_ASM("asm/levels", func_L00_003A2400);

INCLUDE_ASM("asm/levels", func_L00_003A2408);

/* Derives a 16-byte element at index-1 from its index-2 and index-3 neighbours through two bound helpers; original game role is unknown. */
void func_L00_003A2570(Vec16_Fc936841d *object, int index)
{
    char local[16];

    func_00282CF0_9(local, (char *)&object[index - 2], (char *)&object[index - 3]);
    func_00282CC0_15((char *)&object[index - 1], (char *)&object[index - 2], local);
}

/* Derives a value from half of one argument, merges a helper's low byte into a packed word, then scales an input float, doubles a second helper's result and dispatches a seven-argument spawn. */
void func_L00_003A25D8(int p0, int p1, int p2, int p3, float v)
{
    int r;
    float s;
    float t;

    r = func_002A7820_2(p3 >> 1, p3);
    p2 = p2 | (r << 24);
    s = func_002A7878_5(v * 0.75f, v);
    s = s + s;
    t = func_002A7940_2();
    func_00281EB8((char *)p1, 44, p2, 1, s, 0.08f, t);
}

INCLUDE_ASM("asm/levels", func_L00_003A2668);

INCLUDE_ASM("asm/levels", func_L00_003A2B20);

/* straight-line constant-store body (no calls, no branches, no loads) */
void func_L00_003A2B28(char *p)
{
    *(long long *)(p + 112) = 0LL;
    *(int *)(p + 80) = 0;
    *(int *)(p + 88) = 0;
    *(float *)(p + 96) = 1.0f;
    *(float *)(p + 104) = 1.0f;
    *(int *)(p + 84) = 0;
    *(float *)(p + 92) = 1.0f;
    *(int *)(p + 100) = 0;
    *(float *)(p + 108) = 1.0f;
    *(float *)(p + 8) = -2.0f;
    *(float *)(p + 24) = 2.0f;
    *(float *)(p + 40) = -2.0f;
    *(float *)(p + 56) = 2.0f;
    *(float *)(p + 4) = -2.0f;
    *(float *)(p + 36) = 2.0f;
    *(int *)(p + 0) = 0;
    *(int *)(p + 16) = 0;
    *(int *)(p + 32) = 0;
    *(int *)(p + 48) = 0;
    *(float *)(p + 12) = 1.0f;
    *(float *)(p + 28) = 1.0f;
    *(float *)(p + 44) = 1.0f;
    *(long long *)(p + 128) = 280993940374112LL;
    *(float *)(p + 20) = -2.0f;
    *(float *)(p + 52) = 2.0f;
    *(float *)(p + 60) = 1.0f;
}

/* scales a float field by 1/1024, passes a caller word and two 16/48-byte scratch buffers to a four-argument helper, then forwards the second buffer with the scaled float to one helper and two sub-objects of the same parameter to three more helpers */
void func_L00_003A2BB8(char *p, int v, char *a, char *b)
{
    char out0[48];
    char out48[16];
    int w;
    float f;

    f = *(float *)(p + 44) * 0.0009765625f;
    w = v;
    func_002A3F10(p, 1, &w, out0);
    func_00282D30_3(a, out48, f);
    func_00282CC0_4(a, a, b);
    func_00283098_2(a, a, p + 192);
    func_00282CC0_4(a, a, p + 16);
}

INCLUDE_ASM("asm/levels", func_L00_003A2C60);

INCLUDE_ASM("asm/levels", func_L00_003A2EE0);

INCLUDE_ASM("asm/levels", func_L00_003A3B58);

INCLUDE_ASM("asm/levels", func_L00_003A3F30);

INCLUDE_ASM("asm/levels", func_L00_003A41F0);

INCLUDE_ASM("asm/levels", func_L00_003A4480);

INCLUDE_ASM("asm/levels", func_L00_003A48D0);

INCLUDE_ASM("asm/levels", func_L00_003A5368);

/* Runs one helper over the object and a stack buffer, then builds a second call from the object's field 104 sub-object - its field 24 as a float, the constants 2.0f/5.0f/0.6f and a bound data address - and stores the object into field 184 of the returned pointer when it is non-null. */
void func_L00_003A5370(int a0)
{
    int buf[4];
    int s0;
    int v;

    s0 = *(int *)(a0 + 104);
    func_0029FCC0_1(a0, 0, buf);
    v = func_L00_003AB9C0(a0, buf, *(int *)(s0 + 16), D_L00_001A8F20, *(float *)(s0 + 24), 2.0f, 5.0f, 0.6f);
    if (v != 0) {
        *(int *)(v + 184) = a0;
    }
}

INCLUDE_ASM("asm/levels", func_L00_003A53E8);

INCLUDE_ASM("asm/levels", func_L00_003A5DC0);

INCLUDE_ASM("asm/levels", func_L00_003A5FC0);

INCLUDE_ASM("asm/levels", func_L00_003A6538);

INCLUDE_ASM("asm/levels", func_L00_003A6C30);

INCLUDE_ASM("asm/levels", func_L00_003A78B0);

INCLUDE_ASM("asm/levels", func_L00_003A7AF0);

INCLUDE_ASM("asm/levels", func_L00_003A7C30);

INCLUDE_ASM("asm/levels", func_L00_003A7E78);

/* Computes two sub-record addresses of a 192-byte block and hands three of its sub-records to four helpers, threading a second argument through two of them. */
void func_L00_003A8060(char *obj, char *arg)
{
    char *base = obj + 192;
    char *other = obj + 208;
    float one = 1.0f;

    func_00282F20_1(base, (char *)(*(int *)(obj + 104)), one);
    func_00282DD8(other, base, arg);
    func_00282F20_1(other, other, one);
    func_00282DD8(obj + 224, other, base);
}

INCLUDE_ASM("asm/levels", func_L00_003A80F8);

INCLUDE_ASM("asm/levels", func_L00_003A8238);

/* Two size-driven clear loops over one buffer: calls two helpers with a fixed id to obtain a base and an offset, clears 80 records of 112 bytes walking backwards, then clears three fields at offsets 80/84/88 of every 96-byte record up to the buffer end. */
void func_L00_003A8508(char *a0)
{
	int s1;
	int v1;
	int x;
	int i;
	int end;
	short *p;

	s1 = (int)func_L00_0032A088_1(1483, a0);
	v1 = *func_L00_0032A0B8_1(1483, a0);
	x = s1 + v1;
	p = (short *)(s1 + 8866);
	for (i = 79; i >= 0; i--) {
		*p = 0;
		p = p - 56;
	}
	s1 = x;
	end = s1 + 288;
	do {
		*(short *)(s1 + 80) = 0;
		*(int *)(s1 + 84) = 0;
		*(int *)(s1 + 88) = 0;
		s1 = s1 + 96;
	} while (s1 < end);
}

INCLUDE_ASM("asm/levels", func_L00_003A85A0);

INCLUDE_ASM("asm/levels", func_L00_003A8618);

INCLUDE_ASM("asm/levels", func_L00_003A8670);

INCLUDE_ASM("asm/levels", func_L00_003A87D8);

INCLUDE_ASM("asm/levels", func_L00_003A8940);
