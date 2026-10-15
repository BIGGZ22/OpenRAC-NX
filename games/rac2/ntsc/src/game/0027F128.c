#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

typedef unsigned int u32;

typedef unsigned long u64;

extern void func_002FC560(u32,u64);

extern void func_002FC560(unsigned int reg, unsigned long value);

typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

extern s32 D_001A7B90;

extern s32 D_001A7BB8 __attribute__((sda));

extern s32 func_00124418(void);

extern s32 func_001257D0(s32,s32,s32,s32);

typedef struct { u8 before[0x158]; short a,b,c; u8 gap[10]; short d,e; } DisplayState;

extern DisplayState D_001A6480;

extern void func_00125A20(u8 *,short,short,short,short,short);

/* Family c1edda5cc3fa91ec — 148 bytes, levels/2_maktar_nebula and levels/11_joba.

   Write the four-word ring header and advance the resident pointer; when the
   pointer is null, hand the sibling buffer 48 bytes further on to the stop
   helper instead. Every reference reads the global again, and the last store
   addresses it through $gp (a delay-slot form the pinned -G0 profile cannot
   express). */
extern int *D_L02_001B3D08_1 __attribute__((sda));

extern int D_001A742C __attribute__((sda));

extern void D_00126108(void *);

extern volatile int * volatile D_L02_001B3D08_2 __attribute__((sda));

extern volatile int * volatile D_L02_001B3D08_4 __attribute__((sda));

extern char D_001A6D40[];

extern char D_001A6E90[];

s32 func_0027F128(u8 *a0,s32 a1,u8 *a2) {
 s32 out=0,i=0;
 u8 *p;
 if(a1!=0 && a0[0]!=0) {
  p=a0;
  do {
   s32 value=*(signed char *)(a2+p[0]*4+3);
   i++; p++;
   if(value!=0) out+=value;
   if(i==a1) break;
  } while(p[0]!=0);
 }
 return out;
}

extern u8 D_00263B50[];

s32 func_0027F178(u8 *text, s32 length) {
    return func_0027F128(text, length, D_00263B50);
}

extern u8 D_00263EF0[];

s32 func_0027F198(u8 *text, s32 length) {
    return func_0027F128(text, length, D_00263EF0);
}

extern u8 D_00264290[];

s32 func_0027F1B8(u8 *text, s32 length) {
    return func_0027F128(text, length, D_00264290);
}

INCLUDE_ASM("asm/game", func_0027F1D8);

extern int func_0027F1D8(u8 *, int, u8 *);

int func_0027F288(u8 *text, int length) {
    return func_0027F1D8(text, length, D_00263B50);
}

INCLUDE_ASM("asm/game", func_0027F2A8);

INCLUDE_ASM("asm/game", func_0027F530);

extern int func_0027C878(int);
extern void func_0027F2A8(int, int, int, u8 *, int, int, u8 *);

void func_0027F538(int x, int y, int style, u8 *text, int length)
{
    int font = func_0027C878(1);
    func_0027F2A8(x, y, style, text, length, font, D_00263B50);
}

void func_0027F5B8(int x, int y, int style, u8 *text, int length)
{
    int font = func_0027C878(2);
    func_0027F2A8(x, y, style, text, length, font, D_00263EF0);
}


INCLUDE_ASM("asm/game", func_0027F638);

INCLUDE_ASM("asm/game", func_0027F640);

INCLUDE_ASM("asm/game", func_0027F968);

/* func_0027F980 is matched in 0027F980.c. */

/* func_0027FA10 is matched in 0027F980.c. */


INCLUDE_ASM("asm/game", func_0027FAA0);

/* func_0027FAA8 is matched in 0027F980.c. */

/* func_0027FB40 is matched in 0027F980.c. */


/* func_0027FBD8 is matched in 0027F980.c. */

INCLUDE_ASM("asm/game", func_0027FC70);

extern void func_0027FC70(int, int, u8 *, int, int, u8 *);

void func_00280190(int x, int y, u8 *text, int length)
{
    int font = func_0027C878(1);
    func_0027FC70(x, y, text, length, font, D_00263B50);
}

void func_00280200(int x, int y, u8 *text, int length)
{
    int font = func_0027C878(2);
    func_0027FC70(x, y, text, length, font, D_00263EF0);
}


void func_00280270(int x, int y, u8 *text, int length)
{
    int font = func_0027C878(3);
    func_0027FC70(x, y, text, length, font, D_00264290);
}

void func_002802E0(unsigned short *a0,s32 a1,s32 a2,s32 a3,s32 a4,s32 a5,s32 a6,s32 a7,s32 a8) {
 a0[0]=a1;a0[1]=a2;a0[2]=a3;a0[3]=a4;
 a0[4]=a5;a0[5]=a6;a0[8]=a7;a0[9]=a8;
 a0[6]=0;a0[7]=0;a0[10]=0;a0[11]=0;
}

INCLUDE_ASM("asm/game", func_00280318);

INCLUDE_ASM("asm/game", func_00280328);

INCLUDE_ASM("asm/game", func_00280510);

extern void func_00284EA8(void);
extern void func_0027A9F0(void);
extern void func_0027AB40(void);

void func_00280600(void) {
    func_00284EA8();
    func_0027A9F0();
    func_0027AB40();
}

/* func_00280630 -- see src/game/0027F128-g8.c (built with -G8). */

INCLUDE_ASM("asm/game", func_00280660);

INCLUDE_ASM("asm/game", func_00280670);

INCLUDE_ASM("asm/game", func_00280710);

ASM_FUNC("asm/game", func_002808F8);

INCLUDE_ASM("asm/game", func_00280A70);

INCLUDE_ASM("asm/game", func_00280B30);

INCLUDE_ASM("asm/game", func_00280B90);

ASM_FUNC("asm/game", func_00281154);

ASM_FUNC("asm/game", func_002811D4);

ASM_FUNC("asm/game", func_00281214);

INCLUDE_ASM("asm/game", func_00281810);

INCLUDE_ASM("asm/game", func_00281A08);

INCLUDE_ASM("asm/game", func_00281DE0);

INCLUDE_ASM("asm/game", func_00281DE8);

INCLUDE_ASM("asm/game", func_00281E88);

INCLUDE_ASM("asm/game", func_00281EB8);

INCLUDE_ASM("asm/game", func_00282098);

/* calls the measured callee with constants 0x47 and 0x513f1 */
void func_002820A0(void) {
 func_002FC560(0x47,0x513f1UL);
}

void func_002820C8(void)
{
}

INCLUDE_ASM("asm/game", func_002820D0);

INCLUDE_ASM("asm/game", func_002824A0);

INCLUDE_ASM("asm/game", func_002824B8);

INCLUDE_ASM("asm/game", func_00282848);

INCLUDE_ASM("asm/game", func_00282890);

INCLUDE_ASM("asm/game", func_002828B8);

INCLUDE_ASM("asm/game", func_002828C8);

INCLUDE_ASM("asm/game", func_002828F0);

INCLUDE_ASM("asm/game", func_00282900);

INCLUDE_ASM("asm/game", func_00282928);

INCLUDE_ASM("asm/game", func_00282938);

ASM_FUNC("asm/game", func_00282948);

ASM_FUNC("asm/game", func_00282978);

INCLUDE_ASM("asm/game", func_002829A8);

INCLUDE_ASM("asm/game", func_002829D8);

INCLUDE_ASM("asm/game", func_00282A18);

INCLUDE_ASM("asm/game", func_00282A28);

ASM_FUNC("asm/game", func_00282A38);

ASM_FUNC("asm/game", func_00282A60);

ASM_FUNC("asm/game", func_00282A88);

INCLUDE_ASM("asm/game", func_00282AB0);

ASM_FUNC("asm/game", func_00282B50);

ASM_FUNC("asm/game", func_00282B78);

INCLUDE_ASM("asm/game", func_00282BA8);

INCLUDE_ASM("asm/game", func_00282BE0);

INCLUDE_ASM("asm/game", func_00282C08);

INCLUDE_ASM("asm/game", func_00282C10);

ASM_FUNC("asm/game", func_00282C30);

/* 8-BYTE LOT -- bare getters and setters, identical across all 27 levels. */
/* Measured target: byte-identical across all 27 levels. */
f32 func_00282C48(f32 a0) { return __builtin_fabsf(a0); }

INCLUDE_ASM("asm/game", func_00282C50);

INCLUDE_ASM("asm/game", func_00282C58);

INCLUDE_ASM("asm/game", func_00282C68);

INCLUDE_ASM("asm/game", func_00282C78);

/* Clear one aligned 128-bit object through architectural zero. */
void func_00282C88(TI128 *a0) { *a0 = 0; }

INCLUDE_ASM("asm/game", func_00282C90);

INCLUDE_ASM("asm/game", func_00282C98);

INCLUDE_ASM("asm/game", func_00282CA8);

INCLUDE_ASM("asm/game", func_00282CC0);

INCLUDE_ASM("asm/game", func_00282CD8);

INCLUDE_ASM("asm/game", func_00282CF0);

INCLUDE_ASM("asm/game", func_00282D08);

INCLUDE_ASM("asm/game", func_00282D30);

INCLUDE_ASM("asm/game", func_00282D48);

INCLUDE_ASM("asm/game", func_00282D60);

INCLUDE_ASM("asm/game", func_00282D78);

INCLUDE_ASM("asm/game", func_00282D90);

INCLUDE_ASM("asm/game", func_00282DB0);

INCLUDE_ASM("asm/game", func_00282DD8);

INCLUDE_ASM("asm/game", func_00282DF0);

INCLUDE_ASM("asm/game", func_00282E20);

INCLUDE_ASM("asm/game", func_00282E48);

INCLUDE_ASM("asm/game", func_00282E80);

INCLUDE_ASM("asm/game", func_00282EB0);

ASM_FUNC("asm/game", func_00282ED8);

INCLUDE_ASM("asm/game", func_00282F20);

INCLUDE_ASM("asm/game", func_00282F70);

INCLUDE_ASM("asm/game", func_00282FB8);

INCLUDE_ASM("asm/game", func_00283028);

INCLUDE_ASM("asm/game", func_00283098);

INCLUDE_ASM("asm/game", func_002830C0);

INCLUDE_ASM("asm/game", func_002830F0);

INCLUDE_ASM("asm/game", func_00283108);

INCLUDE_ASM("asm/game", func_00283130);

ASM_FUNC("asm/game", func_00283150);

INCLUDE_ASM("asm/game", func_00283180);

INCLUDE_ASM("asm/game", func_00283198);

INCLUDE_ASM("asm/game", func_002831B0);

INCLUDE_ASM("asm/game", func_00283248);

INCLUDE_ASM("asm/game", func_00283360);

INCLUDE_ASM("asm/game", func_00283388);

INCLUDE_ASM("asm/game", func_002833B8);

INCLUDE_ASM("asm/game", func_002833F0);

INCLUDE_ASM("asm/game", func_00283410);

ASM_FUNC("asm/game", func_00283430);

INCLUDE_ASM("asm/game", func_00283658);

INCLUDE_ASM("asm/game", func_00283678);

INCLUDE_ASM("asm/game", func_00283698);

INCLUDE_ASM("asm/game", func_002836E8);

INCLUDE_ASM("asm/game", func_00283738);

ASM_FUNC("asm/game", func_00283788);

INCLUDE_ASM("asm/game", func_002837D0);

INCLUDE_ASM("asm/game", func_00283810);

INCLUDE_ASM("asm/game", func_00283898);

INCLUDE_ASM("asm/game", func_00283958);

INCLUDE_ASM("asm/game", func_002839D0);

INCLUDE_ASM("asm/game", func_00283A58);

INCLUDE_ASM("asm/game", func_00283AF0);

INCLUDE_ASM("asm/game", func_00283B48);

INCLUDE_ASM("asm/game", func_00283B98);

INCLUDE_ASM("asm/game", func_00283BE0);

INCLUDE_ASM("asm/game", func_00283C28);

INCLUDE_ASM("asm/game", func_00283C80);

INCLUDE_ASM("asm/game", func_00283CB8);

INCLUDE_ASM("asm/game", func_00283CC8);

/* SECOND LOT -- 16 to 32 bytes. */
/* integer-to-float conversion; byte-identical across all 27 levels. */
f32 func_00283CE0(s32 a0) { return (f32)a0; }

INCLUDE_ASM("asm/game", func_00283CF0);

INCLUDE_ASM("asm/game", func_00283D00);

INCLUDE_ASM("asm/game", func_00283D38);

INCLUDE_ASM("asm/game", func_00283D80);

ASM_FUNC("asm/game", func_00283DB8);

ASM_FUNC("asm/game", func_00283EB0);

ASM_FUNC("asm/game", func_00283EF0);

INCLUDE_ASM("asm/game", func_00283FE8);

void func_00284000(void) {
    func_00124418();
    if (D_001A7B90 != 0) D_001A7BB8 = 0;
    if (*(s32 *)0x001a7bb8 != 0) func_001257D0(0,0,0x50,1);
    else func_001257D0(0,1,D_001A7B90 != 0 ? 3 : 2,0);
}

INCLUDE_ASM("asm/game", func_00284070);

INCLUDE_ASM("asm/game", func_00284088);

INCLUDE_ASM("asm/game", func_00284268);

INCLUDE_ASM("asm/game", func_00284E18);

void func_00284E40(void) {
    func_00125A20(*(u8 **)0x001a742c,D_001A6480.c,D_001A6480.a,D_001A6480.b,D_001A6480.d,D_001A6480.e);
}

int func_00284E80(void)
{
    extern int func_00125D98(void *);
    return func_00125D98((void *)D_001A742C);
}


/* shared native body of family c1edda5cc3fa91ec */
void func_00284EA8(void)
{
    if (D_L02_001B3D08_1 != 0) {
        *(int *)D_L02_001B3D08_1 = 0x30000009;
        *(int *)((char *)D_L02_001B3D08_1 + 4) = (D_001A742C + 48) & 0x0FFFFFFF;
        *(int *)((char *)D_L02_001B3D08_1 + 8) = 0;
        *(int *)((char *)D_L02_001B3D08_1 + 12) = 0x50000009;
        D_L02_001B3D08_1 = (int *)((char *)D_L02_001B3D08_1 + 16);
    } else {
        D_00126108((void *)(D_001A742C + 48));
    }
}

/* shared native body of family cfd2e0cd2b82a1b7 (default profile) */
void func_00284F40(int a0)
{
    int *p = (int *)D_L02_001B3D08_2;

    if (p == 0)
        return;
    p[0] = 0x30000015;
    if (a0 == 0)
        D_L02_001B3D08_2[1] = (int)D_001A6D40;
    else
        D_L02_001B3D08_2[1] = (int)D_001A6E90;
    D_L02_001B3D08_2[2] = 0;
    D_L02_001B3D08_2[3] = 0x50000015;
    D_L02_001B3D08_4 = D_L02_001B3D08_2 + 4;
}
