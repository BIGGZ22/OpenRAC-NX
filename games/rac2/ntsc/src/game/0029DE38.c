#include "include_asm.h"

/* Measured partial layouts; original type and game roles are unknown. */
typedef unsigned int Rac2Native_abf21065e887d7a9_FallbackU32;

typedef struct Rac2Native_abf21065e887d7a9_FallbackPredicate112Layout {
    unsigned char unknown_00[0x90];
    Rac2Native_abf21065e887d7a9_FallbackU32 flags_90;
    Rac2Native_abf21065e887d7a9_FallbackU32 unknown_94;
    Rac2Native_abf21065e887d7a9_FallbackU32 flags_98;
} Rac2Native_abf21065e887d7a9_FallbackPredicate112Layout;

extern Rac2Native_abf21065e887d7a9_FallbackPredicate112Layout D_00139768;

typedef unsigned char u8;

extern u8 D_L03_001D2540[];

extern float func_00283CE0_4(int a);

extern void Ff2d8e5ad_FUN_002A3110(unsigned char *a, int b, unsigned char *c);

extern void Ff2d8e5ad_FUN_00283130(unsigned char *a, long long v);

extern void Ff2d8e5ad_FUN_002830C0(unsigned char *a, unsigned char *b, unsigned char *c);

extern void Ff2d8e5ad_FUN_00282D30(unsigned char *a, unsigned char *b, float f);

extern void Ff2d8e5ad_FUN_00283098(unsigned char *a, unsigned char *b, unsigned char *c);

extern void Ff2d8e5ad_FUN_00282CC0(unsigned char *a, unsigned char *b, unsigned char *c);

extern float Ff2d8e5ad_FUN_00282E48(unsigned char *a, unsigned char *b);

typedef float f32;

typedef unsigned int u32;

extern void func_00282A60_1 (char *node, int zero, int size);

extern void *D_L00_001B1E00[16];

extern u32 D_L00_001B1E40[16];

extern void func_002A3F10_1(char *, int, int *, char *);

extern void func_00282D30_4(char *, char *, float);

extern void func_00283658(char *, char *);

extern void func_00283788_1(char *, char *, char *);

extern void func_00282CC0_7(char *, char *, char *);

extern void func_002A3F10_2(char *a, int b, char *c, char *d);

extern void func_00282D30_19(char *a, char *b, float f);

extern void func_00283098_6(char *a, char *b, char *c);

extern void func_00282CC0_20(char *a, char *b, char *c);

extern void func_002A3DB8(char *);

extern void func_00282D30_7(char *, char *, float);

extern void func_00283098_3(char *, char *, char *);

extern void func_00282CC0_9(char *, char *, char *);

typedef struct {
    unsigned char key;
    unsigned char reserved01[11];
    int target;
} Row16b52b2850;

typedef struct {
    unsigned char reserved00[32];
    Row16b52b2850 *rows;
} ListObjectb52b2850;

typedef struct {
    short first;
    short second;
} Pairb52b2850;

extern int D_L03_001DE1C0[];

extern ListObjectb52b2850 *D_L03_001DAB80[];

extern Pairb52b2850 D_L03_001DDA00[];

void func_00282A60(unsigned int *dst, unsigned int val, int size);

/* Return whether both flag bit 0x10000 at data offset 0x90 and flag bit 0x04000000 at offset 0x98 are set, retaining the measured early rejection path. */
int func_0029DE38(void)
{
    if ((D_00139768.flags_90 & 0x10000u) == 0 &&
        (D_00139768.flags_98 & 0x04000000u) != 0)
        return 0;
    if ((D_00139768.flags_90 & 0x10000u) != 0 &&
        (D_00139768.flags_98 & 0x04000000u) != 0)
        return 1;
    return 0;
}

INCLUDE_ASM("asm/game", func_0029DEA8);

INCLUDE_ASM("asm/game", func_0029EFC0);

INCLUDE_ASM("asm/game", func_0029EFD0);

INCLUDE_ASM("asm/game", func_0029F0C8);

extern char *D_001B1B60 __attribute__((sda));
extern int D_001B1688 __attribute__((sda));
extern void func_002A0C68(char *, unsigned int);

void func_0029F338(void *object)
{
    char *p = (char *)object;
    if ((unsigned int)p < (unsigned int)D_001B1B60)
        ((unsigned char *)p)[0x20] = 0xFD;
    else
        ((unsigned char *)p)[0x20] = 0xFE;
    *(int *)(p + 0xA0) = D_001B1688 + 2;
    func_002A0C68(p, 0x80807F7Fu);
}

/* shared native body of family d1172b6afb7d978b */
void func_0029F390(u8 *param_1)
{
    if (param_1[66] != 255) {
        u8 *tbl = (u8 *)(*(volatile int *)(param_1 + 36) + 72);

        *(int *)(param_1 + 88) = *(int *)(*(int *)(tbl + (*(volatile u8 *)(param_1 + 66) << 2)) + (param_1[64] << 2) + 28);
        param_1[110] = *(u8 *)(*(int *)(tbl + (*(volatile u8 *)(param_1 + 66) << 2)) + 18);
        param_1[108] = *(u8 *)(*(int *)(tbl + (*(volatile u8 *)(param_1 + 66) << 2)) + 17);
    } else {
        param_1[108] = 255;
        param_1[110] = 0;
        *(int *)(param_1 + 88) = (int)(D_L03_001D2540 + (param_1[64] << 11));
    }

    *(int *)(param_1 + 92) = *(int *)(*(int *)(*(volatile int *)(param_1 + 36) + (param_1[67] << 2) + 72) + (param_1[65] << 2) + 28);
}

INCLUDE_ASM("asm/game", func_0029F450);

INCLUDE_ASM("asm/game", func_0029F528);

/* Picks one of two pointers held at +88 and +92 of the receiver, choosing +92 when the byte at +66 equals 255, then returns a value derived from the scale word at +68: either a halved table lookup, or a weighted blend of two table lookups plus the scale, or an interpolation between a lookup at +92 and a lookup at +88 weighted by one minus the scale. */
float func_0029F530(char *s1)
{
    char *p;
    char *q;
    float t;

    if (*(unsigned char *)(s1 + 66) != 255)
        p = *(char **)(s1 + 88);
    else
        p = *(char **)(s1 + 92);

    if (*(float *)(s1 + 68) == 0.0f)
        return func_00283CE0_4(*(short *)(p + 4)) * 0.0625f;

    if (*(unsigned char *)(s1 + 66) != *(unsigned char *)(s1 + 67) ||
        *(unsigned char *)(s1 + 64) > *(unsigned char *)(s1 + 65))
        return func_00283CE0_4(*(short *)(p + 4)) * 0.0625f + *(float *)(s1 + 68);

    q = *(char **)(s1 + 92);
    t = func_00283CE0_4(*(short *)(p + 4)) * (1.0f - *(float *)(s1 + 68));
    t = t + func_00283CE0_4(*(short *)(q + 4)) * *(float *)(s1 + 68);
    return t * 0.0625f;
}

INCLUDE_ASM("asm/game", func_0029F628);

extern int func_00282A18(int);

void func_0029F648(char *object, int *count, int *value)
{
    char *p = *(char **)(object + 0x24);
    int index = *(unsigned char *)(p + 0x2C);
    if (index != 0) {
        int n;
        index = (int)p + (index << 4);
        n = *(short *)(index + 2);
        *count = n;
        *value = func_00282A18(n * 2);
    } else {
        *count = 0;
        *value = 0;
    }
}

void func_0029F6A0(
    unsigned char *obj, unsigned int index, unsigned char *dest)
{
    unsigned char m0[16];
    unsigned char m1[16];
    unsigned char *base;
    unsigned char *p;
    short n;

    base = *(unsigned char **)(obj + 36);
    p = base + ((unsigned int)*(unsigned char *)(base + 44) << 4);
    p = p + ((index << 4) + 64u);
    n = *(short *)(p + 6);
    if (n < *(short *)(p + 14))
        n = *(short *)(p + 14);
    if (n >= 0)
        Ff2d8e5ad_FUN_002A3110(obj, n + 1, (unsigned char *)0x80000000u);
    Ff2d8e5ad_FUN_00283130(m0, *(long long *)(p));
    Ff2d8e5ad_FUN_00283130(m1, *(long long *)(p + 8));
    *(float *)(m0 + 12) = 1.0f;
    *(float *)(m1 + 12) = 1.0f;
    if (n >= 0) {
        Ff2d8e5ad_FUN_002830C0(m0, m0, (unsigned char *)
            (((unsigned int)*(short *)(p + 6) << 6) + 0x70000000u));
        Ff2d8e5ad_FUN_002830C0(m1, m1, (unsigned char *)
            (((unsigned int)*(short *)(p + 14) << 6) + 0x70000000u));
    }
    Ff2d8e5ad_FUN_00282D30(m0, m0, *(float *)(obj + 44) * 0.0009765625f);
    Ff2d8e5ad_FUN_00282D30(m1, m1, *(float *)(obj + 44) * 0.0009765625f);
    Ff2d8e5ad_FUN_00283098(m0, m0, obj + 192);
    Ff2d8e5ad_FUN_00283098(m1, m1, obj + 192);
    Ff2d8e5ad_FUN_00282CC0(m0, m0, obj + 16);
    Ff2d8e5ad_FUN_00282CC0(m1, m1, obj + 16);
    Ff2d8e5ad_FUN_00282CC0(dest, m0, m1);
    Ff2d8e5ad_FUN_00282D30(dest, dest, 0.5f);
    *(float *)(dest + 12) = Ff2d8e5ad_FUN_00282E48(dest, m0);
}

INCLUDE_ASM("asm/game", func_0029F840);

INCLUDE_ASM("asm/game", func_0029F960);

void func_0029F978(u8 *a0,u32 a1,u8 *a2) {
 u8 *p;
 if(a2[1]!=0) return;
 a2[0]=a1; a2[1]=1;
 *(f32 *)(a2+0x1c)=1.0f;
 *(f32 *)(a2+0x20)=1.0f;
 *(f32 *)(a2+0x24)=1.0f;
 *(f32 *)(a2+0x28)=1.0f;
 p=*(u8 **)(*(u8 **)(*(u8 **)(a0+0x24)+0x1c)+a2[0]*4+4);
 *(u32 *)(a2+4)=p[p[0]+4]*0x40+0x70000000;
 *(u8 **)(a2+8)=*(u8 **)(a0+0x54);
 *(u8 **)(a0+0x54)=a2;
}

/* unlinks a node from a singly linked list (head case and inner-chain case) and calls a release routine on it; does nothing for a null node */
void func_0029F9F0 (char *list, char *node)
{
  char *cur;

  if (node == 0)
    return;

  if (*(char **) (list + 84) == node)
    {
      *(char **) (list + 84) = *(char **) (node + 8);
    }
  else if (*(char **) (list + 84) != 0)
    {
      cur = *(char **) (list + 84);
      while (*(char **) (cur + 8) != 0 && *(char **) (cur + 8) != node)
        cur = *(char **) (cur + 8);
      if (*(char **) (cur + 8) == node)
        *(char **) (cur + 8) = *(char **) (node + 8);
    }
  func_00282A60_1 (node, 0, 64);
}

/* Reserve or reuse an object slot, reset its associated state and return signed slot index */
int func_0029FA88(void *object)
{
    int index;
    for (index = 0; index < 16; ++index) {
        if (D_L00_001B1E00[index] == 0 ||
            D_L00_001B1E00[index] == object) {
            D_L00_001B1E00[index] = object;
            D_L00_001B1E40[index] = 0;
            return index;
        }
    }
    return -1;
}

void func_0029FAE0(void)
{
    int i;
    for (i = 0; i < 16; i++) {
        D_L00_001B1E00[i] = 0;
        D_L00_001B1E40[i] = 0;
    }
}

INCLUDE_ASM("asm/game", func_0029FB20);

/* Copies an int argument into a 64-byte stack local, scales the float at offset 44 of the first pointer by 1/1024, and runs five helpers over the two pointers and that local buffer. */
void func_0029FC20(char *p1, int p2, char *p3)
{
    int n;
    char buf[64];
    float f;

    n = p2;
    f = *(float *)(p1 + 44) * 0.0009765625f;
    func_002A3F10_1(p1, 1, &n, p3);
    func_00282D30_4(p3 + 48, p3 + 48, f);
    func_00283658(buf, p1 + 192);
    func_00283788_1(p3, buf, p3);
    func_00282CC0_7(p3 + 48, p3 + 48, p1 + 16);
}

/* scales a float field by 1/1024, stores an int argument in a stack record, and drives four helper calls with frame and object-field pointers */
void func_0029FCC0(char *a0, int a1, char *a2)
{
    char buf[80];
    float x;

    x = *(float *)(a0 + 44);
    *(int *)(buf + 64) = a1;
    x = x * 0.0009765625f;
    func_002A3F10_2(a0, 1, buf + 64, buf);
    func_00282D30_19(a2, buf + 48, x);
    func_00283098_6(a2, a2, a0 + 192);
    func_00282CC0_20(a2, a2, a0 + 16);
}

/* Loops n times over a 16-byte-strided pointer, scaling the first pointer float at offset 44 by 1/1024 and calling three helpers with that pointer and two offsets of the first argument. */
void func_0029FD48(char *p1, int n, char *p3, char *p4)
{
    float f = *(float *)(p1 + 44) * 0.0009765625f;

    func_002A3DB8(p1);
    if (n > 0) {
        do {
            func_00282D30_7(p4, p4, f);
            func_00283098_3(p4, p4, p1 + 192);
            func_00282CC0_9(p4, p4, p1 + 16);
            p4 += 16;
        } while (--n != 0);
    }
}

INCLUDE_ASM("asm/game", func_0029FDE8);

INCLUDE_ASM("asm/game", func_0029FDF0);

/* shared native body of family b52b2850e37f2b40 */
void func_0029FEE0(void)
{
    int *selected;
    ListObjectb52b2850 *object;
    Row16b52b2850 *row;
    unsigned char *keys;
    unsigned int *dst;
    Pairb52b2850 *pair;
    int *next;

    selected = D_L03_001DE1C0;
    while (*selected >= 0) {
        next = selected + 1;
        object = D_L03_001DAB80[*selected];
        row = object->rows;
        for (;;) {
            keys = (unsigned char *)row;
            dst = (unsigned int *)(row->target & 0x7FFFFFFF);
            if (*keys != 255) {
                do {
                    pair = &D_L03_001DDA00[*keys];
                    if (pair->first != 0) {
                        dst[12] = (dst[12] & 0xFFFFC000u) | pair->first;
                    }
                    keys++;
                    if (pair->second != 0) {
                        dst[16] = (dst[16] & 0xFFFFC000u) | pair->second;
                    }
                    dst += 16;
                } while (*keys != 255);
            }
            if (row->target < 0) {
                goto out;
            }
            row++;
        }
out:
        ;
        selected = next;
    }
}

extern int D_0018C0B0;
extern float D_001B1B90[];
extern void func_002A04E8(int, float *);
extern float func_00283248(float, float);
extern float func_00283180(float);
extern float func_00283198(float);

void func_0029FFB8(void)
{
    float v[2];
    float a;

    func_002A04E8(D_0018C0B0, v);
    a = func_00283248(v[0], v[1]);
    D_001B1B90[0] = func_00283180(a) * 0.14f;
    D_001B1B90[1] = func_00283198(a) * 0.14f;
    D_001B1B90[2] = -0.99f;
}

INCLUDE_ASM("asm/game", func_002A0048);

extern void func_0011AEA0(int);
extern char D_00238F00[];
extern void *D_001B16B0 __attribute__((sda));
extern int D_001B16B4 __attribute__((sda));
extern void func_002A4D88(void *, int);

void func_002A0180(void)
{
    func_0011AEA0(0);
    func_00282B50((void *)0x70003800, D_00238F00, 0x800);
    func_002A4D88(D_001B16B0, D_001B16B4);
}

void func_002A01C8(void)
{
    func_00282A60((unsigned int *)0x70003A00, 0x40000000, 0x3C0);
}

extern void func_00282B50(void *, void *, int);

void func_002A01F0(void)
{
    func_00282B50((void *)((char *)D_L03_001DAB80 - 0x3C0), (void *)0x70003A00, 0x3C0);
}


void func_002A0220(void)
{
    func_00282B50((void *)0x70003A00, (void *)((char *)D_L03_001DAB80 - 0x3C0), 0x3C0);
}


INCLUDE_ASM("asm/game", func_002A0250);

/* func_002A0300 is in 0029DE38-g8.c. */

extern void func_0029FDF0(void);
extern void func_002A0180(void);
extern int func_002A0048(void);
extern int D_001B1B80;

int func_002A0380(void)
{
    int result;
    func_0029FDF0();
    func_002A0180();
    result = D_001B1B80;
    if (result != 0)
        result = func_002A0048();
    return result;
}


/* func_002A03C0 is in 0029DE38-g8.c. */
