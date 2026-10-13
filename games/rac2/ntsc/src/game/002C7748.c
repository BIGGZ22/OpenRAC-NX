#include "include_asm.h"

extern int F250fbfa4_FUN_00282978(char *p);

extern void F250fbfa4_FUN_002B9180(char *p);

extern void F250fbfa4_FUN_00282CC0(char *p0, char *p1, char *p2);

extern int Ff4dd22f3_FUN_00282978(char *p);

extern void Ff4dd22f3_FUN_002B9180(char *p);

extern void Ff4dd22f3_FUN_00282CC0(char *p, char *q, float *v);

extern int Fc40b1162_FUN_00282978();

extern void Fc40b1162_FUN_002B9180();

extern void Fbdd4a4aa_FUN_00300190(char *a, char *b);

extern void Fbdd4a4aa_FUN_00282CC0(char *a, char *b, char *c);

extern void Fbdd4a4aa_FUN_00282D30(char *a, char *b, float f);

extern void Fbdd4a4aa_FUN_002FE8D8(char *a, char *b);

extern int Fbdd4a4aa_FUN_00283D38(unsigned int a, int b, float f);

extern void Fbdd4a4aa_FUN_002B9180(char *a);

/* Frozen Batch A A15-struct-pad-20; acknowledged claim 6d4515f9ca5619f8d02349c6c2ad116dc989f5a4c317c4d31105f9a9271aa080. */
typedef unsigned char BatchA_A15_u8;

typedef unsigned int BatchA_A15_u32;

extern BatchA_A15_u8 D_L15_001FF640[];

/* Frozen Batch A A10-twin-base-32; acknowledged claim b588059f8808a6607867ee41ec367b24cfb85d2f4cc33273a4392842e34233d5. */
typedef unsigned char BatchA_A10_u8;

typedef unsigned int BatchA_A10_u32;

extern BatchA_A10_u8 D_L03_001FF8C0[];

extern BatchA_A10_u8 D_L03_001FFAC0[];

typedef int s32;

typedef unsigned int u32;

extern int func_00288FB8_1 ();

extern void func_0027F5B8 (int v, int p7, long k, int r, int m1);

extern void func_0027FA10 (int v, int p7, long k, int r, int m1);

typedef int BatchA_A12_s32;

typedef struct { BatchA_A12_s32 a; BatchA_A12_s32 b; } BatchA_A12_BatchA24;

extern BatchA_A12_BatchA24 D_L08_001FF980;

typedef unsigned char u8;

/*
 * Disassembled at 0x002cd220
 * Signature: int FUN_002cd220(void)
 * Behavior:
 *   - Saves RA on stack
 *   - Calls func_0027C540(0)
 *   - Calls func_0029C138()
 *   - Calls func_0027C660()
 *   - Restores RA from stack
 *   - Returns 0
 */
extern void func_0027C540(long);

extern void func_0029C138(void);

extern void func_0027C660(void);

void func_0027C540(long);

void func_0027C660(void);

/* 
 * Function: func_002D2EE0
 * 
 * Disassembly summary:
 * - Saves return address on stack
 * - Calls func_0027C540(0) (with zero argument)
 * - Calls func_0029C7A0()
 * - Calls func_0027C660()
 * - Restores return address
 * - Returns 0
 *
 * Notes:
 * - The function follows standard MIPS ABI prologue/epilogue.
 * - Delay slots are filled with NOP or argument setup as seen in disassembly.
 * - No local variables used.
 * - All external function addresses come from the assigned externals list.
 */

extern void func_0027C540(long arg0);

/* External functions declared with exact aliases from task.externals */
extern void func_0027C540(long param_1);

INCLUDE_ASM("asm/game", func_002C7748);

INCLUDE_ASM("asm/game", func_002C7888);

INCLUDE_ASM("asm/game", func_002C7970);

void func_002C7AE8(char *a0)
{
    char local[16];
    char *s1 = a0;
    char *s0;
    int a1;
    int v1;
    char *p;

    if (F250fbfa4_FUN_00282978(a0 + 10) != 0) {
        F250fbfa4_FUN_002B9180(a0);
        return;
    }
    s0 = a0 + 32;
    a1 = *(int *)(s1 + 4);
    v1 = (a1 >> 24) + *(unsigned char *)(s0 + 20);
    *(int *)(s1 + 4) = (a1 & 0x00ffffff) | (v1 << 24);
    p = s1 + 16;
    *(float *)(local + 0) = *(float *)(s1 + 32);
    *(float *)(local + 4) = *(float *)(s0 + 4);
    *(float *)(local + 8) = *(float *)(s0 + 8);
    *(int *)(local + 12) = 0;
    F250fbfa4_FUN_00282CC0(p, p, local);
    *(float *)(s0 + 8) = *(float *)(s0 + 8) - *(float *)(s0 + 12);
    *(unsigned char *)(s1 + 8) = *(unsigned char *)(s1 + 8) + *(unsigned char *)(s0 + 21);
    *(float *)(s1 + 12) = *(float *)(s1 + 12) + *(float *)(s0 + 16);
}

INCLUDE_ASM("asm/game", func_002C7BB8);

void func_002C7D58(char *self)
{
    char *p = self + 32;
    float v[4];
    int a1;

    if (Ff4dd22f3_FUN_00282978(self + 10) != 0)
    {
        Ff4dd22f3_FUN_002B9180(self);
        return;
    }

    a1 = *(int *)(self + 4);
    *(int *)(self + 4) = (a1 & 0xffffff) | (((a1 >> 24) + (int)*(unsigned char *)(p + 20)) << 24);
    v[0] = *(float *)(self + 32);
    v[1] = *(float *)(p + 4);
    v[2] = *(float *)(p + 8);
    v[3] = 0.0f;
    Ff4dd22f3_FUN_00282CC0(self + 16, self + 16, v);
    *(unsigned char *)(self + 8) += *(unsigned char *)(p + 21);
    *(float *)(self + 12) += *(float *)(p + 16);
    *(float *)(p + 8) -= *(float *)(p + 12);
}

INCLUDE_ASM("asm/game", func_002C7E28);

INCLUDE_ASM("asm/game", func_002C7F78);

INCLUDE_ASM("asm/game", func_002C8268);

INCLUDE_ASM("asm/game", func_002C8348);

INCLUDE_ASM("asm/game", func_002C8428);

void func_002C85D8(char *p)
{
    if (Fc40b1162_FUN_00282978(p + 10)) {
        Fc40b1162_FUN_002B9180(p);
    }
}

INCLUDE_ASM("asm/game", func_002C8610);

void func_002C87B0(char *a0)
{
    char local[16];
    char *s1 = a0;
    char *s0;
    int a1;
    int v1;
    char *p;

    if (F250fbfa4_FUN_00282978(a0 + 10) != 0) {
        F250fbfa4_FUN_002B9180(a0);
        return;
    }
    s0 = a0 + 32;
    a1 = *(int *)(s1 + 4);
    v1 = (a1 >> 24) + *(unsigned char *)(s0 + 20);
    *(int *)(s1 + 4) = (a1 & 0x00ffffff) | (v1 << 24);
    p = s1 + 16;
    *(float *)(local + 0) = *(float *)(s1 + 32);
    *(float *)(local + 4) = *(float *)(s0 + 4);
    *(float *)(local + 8) = *(float *)(s0 + 8);
    *(int *)(local + 12) = 0;
    F250fbfa4_FUN_00282CC0(p, p, local);
    *(float *)(s0 + 8) = *(float *)(s0 + 8) - *(float *)(s0 + 12);
    *(unsigned char *)(s1 + 8) = *(unsigned char *)(s1 + 8) + *(unsigned char *)(s0 + 21);
    *(float *)(s1 + 12) = *(float *)(s1 + 12) + *(float *)(s0 + 16);
}

INCLUDE_ASM("asm/game", func_002C8880);

INCLUDE_ASM("asm/game", func_002C8A38);

INCLUDE_ASM("asm/game", func_002C8CA0);

void func_002C8F40(int *obj)
{
    char *a = (char *)obj + 16;
    char *b = (char *)obj + 32;
    char tmp[16];

    Fbdd4a4aa_FUN_00300190(a, a);
    *(int *)(b + 16) = *(int *)(b + 16) - 1;
    if (*(int *)(b + 16) < 0)
        *(int *)(b + 16) = 0;
    *(unsigned char *)((char *)obj + 8) = *(unsigned char *)((char *)obj + 8) + 1;
    Fbdd4a4aa_FUN_00282CC0(a, b, a);
    *(float *)((char *)obj + 12) = *(float *)((char *)obj + 12) + 1.5750000000000000000000e+03f;
    Fbdd4a4aa_FUN_00282D30(b, b, 9.9000000953674316406250e-01f);
    Fbdd4a4aa_FUN_002FE8D8(a, tmp);
    if ((*(int *)((char *)obj + 4) & 0xFF) < 32) {
        Fbdd4a4aa_FUN_00282D30(tmp, tmp, 1.5000000130385160446167e-03f);
        Fbdd4a4aa_FUN_00282CC0(b, b, tmp);
    } else {
        Fbdd4a4aa_FUN_00282D30(tmp, tmp, -7.5000000651925802230835e-04f);
        Fbdd4a4aa_FUN_00282CC0(b, b, tmp);
    }
    *(int *)((char *)obj + 4) = Fbdd4a4aa_FUN_00283D38(*(unsigned int *)((char *)obj + 4) & 0x00FFFFFF, 0, 2.5000000372529029846191e-02f)
        | (*(int *)(b + 16) << 24);
    if (*(int *)(b + 16) == 0)
        Fbdd4a4aa_FUN_002B9180((char *)obj);
}

INCLUDE_ASM("asm/game", func_002C9098);

INCLUDE_ASM("asm/game", func_002C91E0);

INCLUDE_ASM("asm/game", func_002C9218);

INCLUDE_ASM("asm/game", func_002C94C8);

INCLUDE_ASM("asm/game", func_002C96F8);

INCLUDE_ASM("asm/game", func_002C9820);

INCLUDE_ASM("asm/game", func_002C9828);

INCLUDE_ASM("asm/game", func_002C9A40);

extern int D_001F2A30;

int func_002C9A50(void) {
    return D_001F2A30;
}

/* A15-struct-pad-20 freshly exact-verified under family reservation */
void func_002C9A60(void) {
    BatchA_A15_u8 *p = D_L15_001FF640;
    *(BatchA_A15_u32 *)(p + 0xF0) = 0;
    *(BatchA_A15_u32 *)(p + 0x1F0) = 0;
}

/* A10-twin-base-32 freshly exact-verified under family reservation */
void func_002C9A78(void) {
    BatchA_A10_u8 *p = D_L03_001FF8C0;
    *(BatchA_A10_u32 *)(p + 0xF0) = (BatchA_A10_u32)D_L03_001FFAC0;
    *(BatchA_A10_u32 *)(p + 0x1F0) = 1;
}

void func_002C9A98(s32 *a0) {
 s32 *base=a0+3; s32 v;
 do {
  v=--a0[1];
  if(v<=0) v=a0[0];
  a0[1]=v;
 } while(*(s32 *)((u32)base+(v<<2))==0);
}

void func_002C9AD8(s32 *a0) {
 s32 limit=a0[0]; s32 *base=a0+3; s32 v;
 do {
  v=a0[1]+1;
  if(limit<v) v=0;
  a0[1]=v;
 } while(*(s32 *)((u32)base+(v<<2))==0);
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void func_00339770(void *, int);

int func_002C9B18(void) {
    if (D_001A8CFC[0] != 0)
        func_00339770(D_001A8CFC[0] + 0x3CAA8, 0);
    return 0;
}

extern char *D_001A8CFC[] __attribute__((sda));
extern void func_00339770(void *, int);

int func_002C9B50(void) {
    if (D_001A8CFC[0] != 0)
        func_00339770(D_001A8CFC[0] + 0x3CAA8, 1);
    return 0;
}

/* reads two object fields, forms centre plus and minus half of a third, and issues two five-argument draw calls taking an id-derived value and a fixed 64-bit attribute */
int func_002C9B88 (int *p)
{
  int a = p[6];
  int t = p[8] >> 1;
  int diff = a - t;
  int sum = a + t;
  int r;

  r = func_00288FB8_1 (4412);
  func_0027F5B8 (diff, p[7], (long) 0x80f0f0f0, r, -1);
  r = func_00288FB8_1 (4408);
  func_0027FA10 (sum, p[7], (long) 0x80f0f0f0, r, -1);
  return 0;
}

INCLUDE_ASM("asm/game", func_002C9C20);

INCLUDE_ASM("asm/game", func_002C9C60);

INCLUDE_ASM("asm/game", func_002C9C90);

INCLUDE_ASM("asm/game", func_002C9CF8);

INCLUDE_ASM("asm/game", func_002C9E60);

INCLUDE_ASM("asm/game", func_002C9F68);

INCLUDE_ASM("asm/game", func_002CA070);

INCLUDE_ASM("asm/game", func_002CA5D0);

INCLUDE_ASM("asm/game", func_002CA790);

INCLUDE_ASM("asm/game", func_002CA8A8);

extern void func_002CA070(void);
extern unsigned int D_L00_001FEC40[2];
void func_002CAA58(void)
{
    func_002CA070();
    D_L00_001FEC40[0] = 2;
    D_L00_001FEC40[1] = 0;
}


/* A12-struct-2field-24 freshly exact-verified under family reservation */
void func_002CAA88(void) {
    BatchA_A12_BatchA24 *p = &D_L08_001FF980;
    p->a = 3;
    p->b = 0;
}

INCLUDE_ASM("asm/game", func_002CAAA0);

INCLUDE_ASM("asm/game", func_002CAB00);

INCLUDE_ASM("asm/game", func_002CADB0);

INCLUDE_ASM("asm/game", func_002CAF18);

s32 func_002CB4E8(s32 a0) {
 s32 result=0;
 if(a0<0x15 || a0==0x18) result=1;
 return result;
}

INCLUDE_ASM("asm/game", func_002CB510);

INCLUDE_ASM("asm/game", func_002CB648);

INCLUDE_ASM("asm/game", func_002CB7D0);

INCLUDE_ASM("asm/game", func_002CB820);

INCLUDE_ASM("asm/game", func_002CB8A0);

INCLUDE_ASM("asm/game", func_002CB950);

INCLUDE_ASM("asm/game", func_002CBA58);

INCLUDE_ASM("asm/game", func_002CBAE0);

INCLUDE_ASM("asm/game", func_002CBC58);

INCLUDE_ASM("asm/game", func_002CC490);

INCLUDE_ASM("asm/game", func_002CC528);

INCLUDE_ASM("asm/game", func_002CC550);

int func_002CC690(void) {
    return 0;
}

int func_002CC698(void) {
    return 0;
}

s32 func_002CC6A0(u8 *p) { *(s32 *)(p + 0x44) = -1; return 0; }

INCLUDE_ASM("asm/game", func_002CC6B0);


INCLUDE_ASM("asm/game", func_002CCF68);

INCLUDE_ASM("asm/game", func_002CD0E8);

int func_002CD220(void)
{
  func_0027C540(0);
  func_0029C138();
  func_0027C660();
  return 0;
}

INCLUDE_ASM("asm/game", func_002CD250);

INCLUDE_ASM("asm/game", func_002CD3C0);

INCLUDE_ASM("asm/game", func_002CD4E0);
