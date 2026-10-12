#include "include_asm.h"

extern void func_L00_0042BCF8_1(void *p);

extern void func_0034B070(void *p);

extern void func_L00_0042BFF8_7(void *p);

extern void func_00337BD8(void *p);

extern char *F476e86a4_FUN_00335E10(char *p);

extern void F476e86a4_FUN_003364C8(char *p, int a, int b);

extern void F476e86a4_FUN_00336508(char *p, int a, int b);

extern void F476e86a4_FUN_00336CE0(char *p, int a);

/* Qwen draft (branch qwen/autonomy, 56f9a59b), checked by tools/check.py */
void func_00338A60(int);

/* Qwen draft (branch qwen/autonomy, 576906e6), checked by tools/check.py */
void func_00338F58(int);

extern char F45821cfb_D_001C2740[];

extern void F45821cfb_FUN_00282C88(char *);

extern char F0a76d85b_D_001B5200[];

extern void F0a76d85b_FUN_00283360(char *p);

extern void F0a76d85b_FUN_00279D18(void);

typedef int s32;

/* lot15 -- first CALL-BEARING body: the call family is now open. */
extern void func_0011AAD0(s32);

/* lot15 -- two twins of the first call-bearing body (same form, 32 bytes).
   The matched wrappers use a `nop` delay slot and return 1; the slot alone does not establish the callee parameter list. */
extern void func_001338C8();

typedef unsigned char u8;

extern void func_001338F0(s32, s32, s32, s32, s32);

extern s32 func_00133890(void);

extern s32 func_0011AEA0(s32);

extern s32 func_0011AFE0(s32 *, s32);

extern s32 func_0011AFC0(s32);

extern s32 func_00133930(s32, s32);

extern volatile unsigned int D_L00_001AEB10 __attribute__((sda));

typedef struct { int pos; int used; int size; } Ring;

INCLUDE_ASM("asm/game", func_0034CEA0);

INCLUDE_ASM("asm/game", func_0034DB98);

INCLUDE_ASM("asm/game", func_0034DFD8);

/* walks 26 consecutive 72-byte records calling one leaf per record, then calls four more leaves on fixed offsets of the same object and returns the object pointer */
unsigned char *func_0034DFE0(unsigned char *a0)
{
    unsigned char *p;
    int i;

    for (i = 25, p = a0; i != -1; i--) {
        func_L00_0042BCF8_1(p);
        p += 72;
    }
    func_L00_0042BCF8_1(a0 + 1872);
    func_0034B070(a0 + 1952);
    func_L00_0042BFF8_7(a0 + 7472);
    func_00337BD8(a0 + 7532);
    func_00337BD8(a0 + 8124);
    return a0;
}

INCLUDE_ASM("asm/game", func_0034E070);

void func_0034E218(void)
{
}

void func_0034E220(char *a0, int a1, int a2, int a3, int t0)
{
    char *base = a0 + a1 * 72;
    float *p;
    char *vt;

    p = (float *)F476e86a4_FUN_00335E10(base);
    p[0] = (float)a2;
    p = (float *)F476e86a4_FUN_00335E10(base);
    p[1] = (float)a3;
    F476e86a4_FUN_003364C8(base, t0, t0);
    F476e86a4_FUN_00336508(base, t0, t0);
    F476e86a4_FUN_00336CE0(base, t0 << 24);
    vt = *(char **)(base + 48);
    (*(void (**)(char *))(vt + 12))(base + *(short *)(vt + 8));
}

INCLUDE_ASM("asm/game", func_0034E2E0);

INCLUDE_ASM("asm/game", func_0034E478);

void func_0034E4B8(int param_1)
{
  func_00338A60(param_1 + 0x1d6c);
}

void func_0034E4D8(int param_1)
{
  func_00338F58(param_1 + 0x1d6c);
  return;
}

INCLUDE_ASM("asm/game", func_0034E4F8);

void *func_0034E5B8(void *object) {
    return object;
}

INCLUDE_ASM("asm/game", func_0034E5C0);

INCLUDE_ASM("asm/game", func_0034EB20);

void func_0034EBE0(void)
{
    float *p = (float *)F45821cfb_D_001C2740;

    p[224] = 0.4f;
    p[225] = 0.8f;
    p[226] = 1.2f;
    *(int *)&p[227] = 0;
    p[228] = 0.577f;
    p[229] = 0.577f;
    p[230] = -0.577f;
    *(int *)&p[231] = 0;
    F45821cfb_FUN_00282C88((char *)&p[232]);
    F45821cfb_FUN_00282C88((char *)&p[236]);
}

void func_0034EC70(void)
{
    char *p = F0a76d85b_D_001B5200;

    *(int *)(p + 320) = 0;
    *(int *)(p + 324) = 0;
    *(int *)(p + 328) = 0;
    F0a76d85b_FUN_00283360(p + 880);
    F0a76d85b_FUN_00279D18();
}

INCLUDE_ASM("asm/game", func_0034ECA8);

INCLUDE_ASM("asm/game", func_0034ECB0);

INCLUDE_ASM("asm/game", func_0034EDB0);

INCLUDE_ASM("asm/game", func_0034EE78);

INCLUDE_ASM("asm/game", func_0034EF58);

INCLUDE_ASM("asm/game", func_0034F048);

void func_0034F3B8(void) {
    func_0011AAD0(1);
}

extern char *D_001B23CC[] __attribute__((sda));
extern int func_0034FAE8(u8 *);

int func_0034F3D8(void) {
    return func_0034FAE8((u8 *)(D_001B23CC[0] + 0xD9100));
}

INCLUDE_ASM("asm/game", func_0034F408);

INCLUDE_ASM("asm/game", func_0034F690);

extern char D_001AE820[];
extern void func_0026FEB8(void *, int);

void func_0034F780(int value) {
    func_0026FEB8(D_001AE820, value);
}

extern char *D_001B23CC[] __attribute__((sda));
extern void func_0034FAF8(int *);

void func_0034F7A8(void) {
    func_0034FAF8((int *)(D_001B23CC[0] + 0xD9100));
}

INCLUDE_ASM("asm/game", func_0034F7D8);

s32 func_0034F898(void) {
    func_001338C8();
    return 1;
}

INCLUDE_ASM("asm/game", func_0034F8B8);

void func_0034F8C0(u8 *a0) {
    func_001338F0(*(s32 *)(a0+0x48), *(s32 *)(a0+0x4c) / 1024 * 1024,
                *(s32 *)(a0+0x5c), *(s32 *)(a0+0x14), *(s32 *)(a0+0x18));
    *(s32 *)a0 = 2;
}

void func_0034F918(u8 *a0) {
    func_00133890();
    *(volatile s32 *)(a0 + 0x5c) = 0;
    *(volatile s32 *)(a0 + 0x00) = 0;
    *(volatile s32 *)(a0 + 0x30) = 0;
    *(volatile s32 *)(a0 + 0x38) = 0;
    *(volatile s32 *)(a0 + 0x3c) = 0;
    *(volatile s32 *)(a0 + 0x44) = 0;
    *(volatile s32 *)(a0 + 0x50) = 0;
    *(volatile s32 *)(a0 + 0x58) = 0;
}

void func_0034F960(s32 *a0,s32 *a1,s32 *a2,s32 *a3,s32 *a4) {
 s32 offset,n;
 if(a0[0]==0) {
  if(a0[1]!=4) {
   *a1=(s32)((u8 *)a0+(a0[12]+8));
   *a2=0x28-a0[12]; *a3=a0[13]; *a4=a0[16];
   return;
  }
  *a1=a0[13]; *a2=a0[16];
  clear: *a3=0; *a4=0; return;
 }
 n=a0[16]-a0[15];offset=a0[14];
 if(a0[16]-offset>=n) {
  *a1=a0[13]+offset; *a2=n; goto clear;
 }
 *a1=a0[13]+offset;
 *a2=a0[16]-a0[14]; *a3=a0[13];
 *a4=n-(a0[16]-a0[14]);
}

void func_0034FA30(s32 *a0,s32 a1) {
    if(a0[0]==0) {
        if(a0[1]!=4) {
            s32 space=40-a0[12];
            s32 take=a1;
            if(space<a1) take=space;
            a0[12]+=take;
            if(a0[12]>39) a0[0]=1;
            a1-=take;
        } else a0[0]=1;
    }
    a0[16]=a0[16]/1024*1024;
    a0[14]=(a0[14]+a1)%a0[16];
    a0[15]+=a1;
    a0[17]+=a1;
}

/* true when the counter has reached 0x1000; byte-identical across all 27 levels. */
s32 func_0034FAE8(u8 *a0) { return *(s32 *)(a0 + 0x50) >= 0x1000; }

extern void func_0034FBC8(void *);

void func_0034FAF8(int *object) {
    if (object[0] != 0)
        func_0034FBC8(object);
}

void func_0034FB20(u8 *a0, s32 a1, s32 a2, s32 a3) {
    s32 dma[4], id;
    ((void (*)(s32))func_0011AEA0)(0);
    dma[0] = a1; dma[1] = *(s32 *)(a0+0x48); dma[2] = a2; dma[3] = 0;
    do { id = func_0011AFE0(dma,1); } while (id == 0);
    while (func_0011AFC0(id) >= 0) {}
    func_00133930(a2,a3);
}

INCLUDE_ASM("asm/game", func_0034FBC8);

INCLUDE_ASM("asm/game", func_0034FE18);

INCLUDE_ASM("asm/game", func_0034FFC0);

INCLUDE_ASM("asm/game", func_00350190);

INCLUDE_ASM("asm/game", func_003501E0);

/* A19-sda-scalar-store-16 freshly exact-verified under family reservation */
void func_00350230(void) { D_L00_001AEB10 = 0; }

INCLUDE_ASM("asm/game", func_00350240);

INCLUDE_ASM("asm/game", func_00350378);

INCLUDE_ASM("asm/game", func_00350460);

void func_00350590(u8 *a0) {
 a0+=0x50000;
 *(s32 *)(a0+8)=0x50000;
 *(s32 *)a0=0;
 *(s32 *)(a0+4)=0;
}

void func_003505A8(void)
{
}

s32 func_003505B0(u8 *a0,u8 **a1) {
 s32 *p=(s32 *)(a0+0x50000);
 s32 n=p[2]-p[1];
 if(n!=0) *a1=a0+p[0];
 return n;
}

void func_003505E0(char *base, int n) {
    Ring *r = (Ring *)(base + 0x50000);
    int avail = r->size - r->used;
    if (n < avail) avail = n;
    r->pos = (r->pos + avail) % r->size;
    r->used += avail;
}

s32 func_00350628(u8 *a0,s32 *a1) {
    u8 *p=a0+0x50000;
    s32 used=*(s32 *)(p+4);
    if(used!=0) {
        s32 size=*(s32 *)(p+8);
        *a1=(s32)a0+((*(s32 *)p-used)+size)%size;
    }
    return *(s32 *)(p+4);
}

s32 func_00350670(u8 *a0,s32 a1) {
 s32 original,value;
 a0+=0x50000;
 value=*(s32 *)(a0+4);original=value;
 if(a1<value) value=a1;
 *(s32 *)(a0+4)=original-value;
 return value;
}

/* stores two words and returns 1; byte-identical across all 27 levels. */
s32 func_00350698(u8 *a0, s32 a1, s32 a2) { *(s32 *)(a0 + 0x4) = a1; *(s32 *)a0 = a2; return 1; }
