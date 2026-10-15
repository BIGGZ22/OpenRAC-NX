#include "include_asm.h"

typedef int s32;

typedef unsigned char u8;

/* lot10: privately byte-gated bodies. */
extern s32 func_00126470(u8 *, s32, s32, s32, s32, s32, s32, s32);

extern s32 func_0011AEA0(s32);

extern s32 func_00126730(u8 *, u8 *);

extern s32 func_001244B8(s32, s32);

/* Family d50cdefed4015bae — 364 bytes, a 16384-byte block loop that builds two
   four-word GS packets per iteration and calls the measured SDK helper at
   0x00126288 in between.

   The resident write pointer at 0x001B3D08 is read ten times and stored three
   times.  Two authored views of that one measured cell are needed and are the
   qualified form here:
     * the read view is volatile, so every use is a fresh load -- the retail
       body re-loads the cell before every field store (aliasing through the
       loaded pointer is not enough to force it: without the qualifier cc1
       forwards a stored value into the next two reads);
     * the write view is an ordinary pointer, so the store written before the
       call is moved into the `jal` delay slot, which is where retail keeps it
       (a volatile store is left in place).
   The bare symbols keep the address materialisation a per-site assembler job
   (`lui`+`lw` mid-stream, the single `$gp` form in a delay slot).

   -G0 promotion (agent f02, 2026-10-07): under -G8 these two declarations need
   no attribute, because -G8 alone puts the cell in small data.  Under -G0 cc1
   would otherwise model each access as an explicit `lui`+`lw` pair, hoist the
   base into a callee-saved register and reuse it, which changes both the
   register allocation and the body size.  The measured `sda` attribute puts
   cc1 back on the one-instruction model, so gas expands every site exactly as
   the retail build did: absolute `lui`+`lw` in ordinary flow, the single `$gp`
   form inside the `.set nomacro` region cc1 emits for the `jal` delay slot.
   Nothing else about the body changed. */

extern int *volatile D_L02_001B3D08_3 __attribute__((sda));

extern int *D_L02_001B3D08_5 __attribute__((sda));

extern void D_00126288(int *a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7);

typedef unsigned int u32;

extern int D_L03_001B31A0 __attribute__((sda));

extern int D_L03_001B31A4 __attribute__((sda));

extern void func_0011A950(int, int);

extern void func_0011B658(int);

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_74a5d71aacb394c6_u32;

typedef struct {
    Rac2Native_74a5d71aacb394c6_u32 first[64];
    Rac2Native_74a5d71aacb394c6_u32 second[64];
    unsigned char gap[0x20];
    int count;
    Rac2Native_74a5d71aacb394c6_u32 current;
} Rac2Native_74a5d71aacb394c6_ExtraIndexState48;

extern int D_L03_002325D0[];

void func_002FC8D0(u8 *a0, s32 a1, s32 a2) {
    u8 image[112];
    s32 remain = (a2 + 0x3fff) & ~0x3fff;
    s32 src = 0, dst = 0;
    for (; remain > 0; remain -= 0x4000) {
        func_00126470(image, (s32)(short)((a1+src)>>8), 1, 1, 0, 0, 64, 64);
        src += 0x4000;
        func_0011AEA0(0);
        func_00126730(image,a0+dst);
        dst += 0x3000;
        func_001244B8(0,0);
    }
}

/* shared native body of family d50cdefed4015bae (default profile) */
void func_002FC990(int a0, int a1, int a2)
{
    int s0 = (a2 + 16383) & -16384;
    int s6 = 0;
    int s5 = 0;

    while (s0 > 0) {
        *(int *)((char *)D_L02_001B3D08_3 + 0) = 0x10000006;
        *(int *)((char *)D_L02_001B3D08_3 + 4) = 0;
        *(int *)((char *)D_L02_001B3D08_3 + 8) = 0;
        *(int *)((char *)D_L02_001B3D08_3 + 12) = 0x50000006;
        {
            int *next = (int *)((char *)D_L02_001B3D08_3 + 16);
            D_L02_001B3D08_5 = next;
            D_00126288(next, (short)((a1 + s6) >> 8), 1, 1, 0, 0, 64, 64);
        }
        s6 += 16384;
        s0 -= 16384;
        {
            int *p = D_L02_001B3D08_3;
            int off = a0 + s5;
            s5 += 12288;
            D_L02_001B3D08_5 = (int *)((char *)p + 96);
            *(int *)((char *)p + 96) = 0x30000300;
            *(int *)((char *)D_L02_001B3D08_3 + 4) = off;
            *(int *)((char *)D_L02_001B3D08_3 + 8) = 0;
            *(int *)((char *)D_L02_001B3D08_3 + 12) = 0x50000300;
            D_L02_001B3D08_5 = (int *)((char *)D_L02_001B3D08_3 + 16);
        }
    }
}

INCLUDE_ASM("asm/game", func_002FCB00);

/* The guard reads the cell at 0x001B22C0/0x001B22C4: the read view is volatile (every use is a
   fresh load), and the write of the initialiser's result is an ordinary store, which is what
   leaves the first call's `lui` in the `bne` delay slot instead of duplicating it.  Both cells
   are reached with retail's one-instruction macro form, so they carry the `sda` attribute. */
extern int D_001B22C0 __attribute__((sda));

extern int D_001B22C4 __attribute__((sda));

extern void func_002FCE18(void);

extern void func_002FCF18(void);

extern int func_0011A930(int, void (*)(void), int);

extern void func_0011B6C0(int);

void func_002FCD18(void)
{
    if (D_001B22C0 != 0) {
        return;
    }
    if (D_001B22C4 != 0) {
        return;
    }
    if ((*(volatile unsigned int *)0x1000E010 & 0x20000) == 0) {
        *(unsigned int *)0x1000E010 = 0x20000;
    }
    D_001B22C0 = func_0011A930(1, func_002FCE18, 0);
    D_001B22C4 = func_0011A930(15, func_002FCF18, 0);
    func_0011B6C0(1);
}

/* shared native body of family 349daa693c44d158 */
void func_002FCDA8(void)
{
    if ((*(volatile u32 *)0x1000E010 & 0x20000) != 0) {
        *(volatile u32 *)0x1000E010 = 0x20000;
    }
    func_0011A950(1, D_L03_001B31A0);
    func_0011A950(15, D_L03_001B31A4);
    func_0011B658(1);
    D_L03_001B31A0 = 0;
    D_L03_001B31A4 = 0;
}

ASM_FUNC("asm/game", func_002FCE18);

INCLUDE_ASM("asm/game", func_002FCF18);

#define MACRO_ADDR __attribute__((section(".sdata")))

/*
 * The VU1 chain state. r1_D_00161000 (the write pointer), r1_D_00161010 (the
 * buffer index) and the two limits r1_D_0015F718/r1_D_0015F71C are all reached
 * with retail's one-instruction macro form, so they are MACRO_ADDR: the
 * assembler expands them through $at, or through $gp where the access
 * lands in a delay slot.
 */
extern int *r1_D_00161000 MACRO_ADDR;

/*
 * Append one quadword to the packet at r1_D_00161000. This one wants the
 * pointer typed as a quadword: stepping an `int *` by 4 folds the copy
 * retail keeps (`daddu $5,$2,$0`) into the increment and comes out one
 * instruction short, so the same symbol is declared a second time under
 * a quadword type.
 */
typedef struct {
    int w[4];
} Qword_r002FCF90;

extern Qword_r002FCF90 *r1_D_00161000_q __asm__("r1_D_00161000") MACRO_ADDR;

/* rac1-decomp src/game/vuchain.c: func_00235290 */
void func_002FCF90(int arg0) {
    Qword_r002FCF90 *p = r1_D_00161000_q++;
    p->w[0] = arg0 + 0x90000000;
    p->w[1] = 0;
    p->w[2] = 0;
    p->w[3] = 0;
}

INCLUDE_ASM("asm/game", func_002FCFC8);

INCLUDE_ASM("asm/game", func_002FD1A0);

INCLUDE_ASM("asm/game", func_002FD1B8);

INCLUDE_ASM("asm/game", func_002FD828);

INCLUDE_ASM("asm/game", func_002FDB40);

INCLUDE_ASM("asm/game", func_002FDBD8);

INCLUDE_ASM("asm/game", func_002FDF40);

INCLUDE_ASM("asm/game", func_002FE5A0);

extern void func_002FE8D8(void *, float *);
extern void func_00282D30(float *, float *, float);
extern void func_00282CF0(float *, float *, float *);

void func_002FE880(void *object, float *destination, float scale)
{
    float vector[4];
    func_002FE8D8(object, vector);
    func_00282D30(vector, vector, scale);
    func_00282CF0(destination, destination, vector);
}

INCLUDE_ASM("asm/game", func_002FE8D8);

INCLUDE_ASM("asm/game", func_002FE9D0);

extern char D_0021F9C0[];

void *func_002FEC50(int index) {
    void *result = 0;
    if ((unsigned int)index < 64u)
        result = D_0021F9C0 + index * 0x50;
    return result;
}

extern void func_0029F978(unsigned char *, unsigned int, unsigned char *);

int func_002FEC78(unsigned char *object, unsigned int flags)
{
    char *p = D_0021F9C0;
    int i = 0;
    do {
        if (*(unsigned char **)(p + 0x40) != 0) {
            i++;
            p += 0x50;
            continue;
        } else {
            *(int *)(p + 4) = -1;
            *(unsigned char **)(p + 0x40) = object;
            p[3] = 0;
            p[0] = 0;
            *(int *)(p + 0xC) = 0;
            *(int *)(p + 8) = 0;
            *(int *)(p + 0x10) = 0;
            *(int *)(p + 0x14) = 0;
            *(int *)(p + 0x18) = 0;
            *(int *)(p + 0x1C) = 0;
            *(int *)(p + 0x30) = 0;
            *(int *)(p + 0x34) = 0;
            *(int *)(p + 0x38) = 0;
            *(int *)(p + 0x3C) = 0;
            p[2] = 0;
            *(int *)(p + 0x20) = 0;
            *(int *)(p + 0x24) = 0;
            *(int *)(p + 0x28) = 0;
            *(int *)(p + 0x2C) = 0;
            p[1] = 0;
            func_0029F978(object, flags, (unsigned char *)p);
            break;
        }
    } while (i < 64);
    return i;
}

extern void func_0029F9F0(char *, char *);
void func_002FED28(unsigned int index) { if (index < 64u) { char *p = D_0021F9C0 + index * 0x50; func_0029F9F0(*(char **)(p + 0x40), p); *(char **)(p + 0x40) = 0; } }

INCLUDE_ASM("asm/game", func_002FED78);

INCLUDE_ASM("asm/game", func_002FED90);

extern char D_002212E0[];

void func_002FEE50(int index) {
    int *p;
    if (index < 0) return;
    p = (int *)((unsigned int)D_002212E0 + (index << 5));
    p[7] = 0;
}

INCLUDE_ASM("asm/game", func_002FEE70);

INCLUDE_ASM("asm/game", func_002FF0B0);

INCLUDE_ASM("asm/game", func_002FF0D8);

void func_002FF288(void)
{
}

extern void func_0029F0C8(void *, int);
extern void func_00282A60(unsigned int *, unsigned int, int);
extern void func_002A0F48(void *);
extern char *ref_L00_002FF290_00 __attribute__((sda));
extern char *ref_L00_002FF290_01 __attribute__((sda));
void func_002FF290(char *object, int kind)
{
    char *data;
    func_0029F0C8(object, kind);
    data = ref_L00_002FF290_01 + (((int)object - (int)ref_L00_002FF290_00) >> 8) * 128;
    *(char **)(object + 0x68) = data;
    func_00282A60((unsigned int *)data, 0, 128);
    if ((*(unsigned short *)(object + 0x34) & 4) == 0)
        func_002A0F48(object);
}

INCLUDE_ASM("asm/game", func_002FF300);

extern char *D_001B2348 __attribute__((sda));
extern void func_002FF290(char *, int);

void func_002FF3F8(void)
{
    char *object;
    func_002FF290(D_001B2348, 0x3EF);
    object = D_001B2348;
    *(unsigned char *)(object + 0x30) = 255;
    *(unsigned short *)(object + 0x34) |= 0x43;
    *(int *)(object + 0x98) = 0;
    *(float *)(object + 0x18) = 5.0f;
    *(float *)(object + 0x14) = 5.0f;
    *(float *)(object + 0x10) = 5.0f;
}

INCLUDE_ASM("asm/game", func_002FF450);

INCLUDE_ASM("asm/game", func_002FF968);

INCLUDE_ASM("asm/game", func_002FFB68);

INCLUDE_ASM("asm/game", func_002FFCF8);

INCLUDE_ASM("asm/game", func_002FFD78);

INCLUDE_ASM("asm/game", func_002FFFE0);

INCLUDE_ASM("asm/game", func_00300048);

INCLUDE_ASM("asm/game", func_00300180);

INCLUDE_ASM("asm/game", func_00300190);

INCLUDE_ASM("asm/game", func_003001E0);



s32 func_00300280(s32 *a0,s32 a1) {
 s32 out=-1,i=0,flag=1; s32 *p=a0;
 for(;i<64;i++,p++) {
  if(*p==0) {
   *p=a1;
   a0[0x89]=flag;
   a0[0x88]++;
   out=i;
   p[0x40]=0;
   break;
  }
 }
 return out;
}

/* Clear corresponding indexed words and decrement a record count */
void func_00300300(Rac2Native_74a5d71aacb394c6_ExtraIndexState48 *state, int index) {
    state->first[index] = 0;
    state->second[index] = 0;
    if (--state->count == 0) {
        state->current = 0;
    }
}

INCLUDE_ASM("asm/game", func_00300330);

INCLUDE_ASM("asm/game", func_003004A8);

typedef struct {
    int padding;
    float value;
    int reserved[2];
} Record003004B0;

typedef struct {
    Record003004B0 records[64];
    int enabled;
    int index;
} State003004B0;

extern State003004B0 D_00220DC0;

float func_003004B0(float value) {
    float result = value;
    if (D_00220DC0.enabled != 0)
        result = D_00220DC0.records[D_00220DC0.index].value;
    return result;
}

/* shared native body of family 8a42bc68419c7402 */
int func_003004E0(int value)
{
    int i;
    int found = 0;

    for (i = 0; i < D_L03_002325D0[64]; i++)
    {
        if (D_L03_002325D0[i] == value)
        {
            found = 1;
            break;
        }
    }
    return found;
}

/* func_00300540 : dispatch: weapon category selected by the identifier */
s32 func_00300540(s32 a0) {
    s32 v = 0;
    if (a0 == 0 || a0 == 14 || a0 == 13) v = 2;
    else if (a0 == 1 || a0 == 3 || a0 == 11 || a0 == 12) v = 1;
    return v;
}

INCLUDE_ASM("asm/game", func_00300598);
