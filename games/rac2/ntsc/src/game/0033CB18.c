#include "include_asm.h"

extern void Fc558ca70_FUN_0033AE28(char *p);

extern void Fc558ca70_FUN_0033B018(char *p, float a, float b);

extern void Fc558ca70_FUN_002E5E58(int a, int b, int c);

extern volatile unsigned char Fc558ca70_D_001A7B95 __attribute__((sda));

extern volatile unsigned char Fc558ca70_D_001A7B94 __attribute__((sda));

extern void Ffb202470_FUN_0033AB50(char *p);

extern void F5979c03f_FUN_0033AE28 (char *object);

extern void F5979c03f_FUN_0033B018 (char *object, float x, float y);

extern int F5979c03f_FUN_002E5E58 (int a, int b, int c);

extern void Fb7feb893_FUN_0033AE28(char *p);

extern void Fb7feb893_FUN_0033B018(char *p, float a, float b);

extern void Fb7feb893_FUN_002E5E58(int a, int b, int c);

extern volatile unsigned char Fb7feb893_D_001A7BB2 __attribute__((sda));

extern volatile unsigned char Fb7feb893_D_001A7BB3 __attribute__((sda));

extern void Fb1bbe20c_FUN_0033AE28(char *p);

extern void Fb1bbe20c_FUN_0033B018(char *p, float x, float y);

extern int Fb1bbe20c_FUN_002E5E58(int a, int b, int c);

extern volatile unsigned char Fb1bbe20c_D_001A7BB2 __attribute__((sda));

void* func_0033E170(void* param_1);

extern void func_0033AB50(int);

extern unsigned long long func_00347EA0(unsigned long long);

extern int func_0033E348(int);

int func_0033CB18(char *a0, int a1)
{
    char *p = a0 + 8;
    float *q;
    int v;

    Fc558ca70_FUN_0033AE28(p);
    q = *(float **)(a0 + 684);
    Fc558ca70_FUN_0033B018(p, q[0], q[1]);
    if (a1 & 0x1000) {
        Fc558ca70_FUN_002E5E58(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x4000) {
        Fc558ca70_FUN_002E5E58(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x40) {
        Fc558ca70_FUN_002E5E58(4, 0, 0);
        switch (*(int *)(a0 + 680)) {
        case 0:
            Fc558ca70_D_001A7B95 = Fc558ca70_D_001A7B95 == 0;
            break;
        case 1:
            Fc558ca70_D_001A7B94 = Fc558ca70_D_001A7B94 == 0;
            break;
        }
    }
    return (a1 >> 6) & 1;
}

INCLUDE_ASM("asm/game", func_0033CC20);

char *func_0033CD80(char *object)
{
    extern void func_003363D0(void *);
    func_0033AB50((int)(object + 8));
    func_003363D0(object + 0x2AC);
    func_003363D0(object + 0x2F4);
    return object;
}

INCLUDE_ASM("asm/game", func_0033CDC0);



INCLUDE_ASM("asm/game", func_0033D028);

INCLUDE_ASM("asm/game", func_0033D2C0);

char *func_0033D440(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033D470);



int func_0033D5A0 (char *object, int flags)
{
  volatile unsigned char *g = (volatile unsigned char *) 0x1A7B96;
  float *vec;
  int discarded;

  F5979c03f_FUN_0033AE28 (object + 8);
  vec = *(float **) (object + 684);
  F5979c03f_FUN_0033B018 (object + 8, vec[0], vec[1]);

  if ((flags & 0x40) && *(int *) (object + 680) == 0)
    {
      discarded = F5979c03f_FUN_002E5E58 (4, 0, 0);
      (void) discarded;
      *g = (*g < 1);
    }

  return (flags >> 6) & 1;
}

typedef struct { int ids[2]; } SolTextPair;
extern SolTextPair D_L00_001AE110;
extern char D_L00_001AE118[];
/* Resident byte at 0x001A7BB2 is shared by all programs. */
extern void func_0033AF48(void *);
extern char *func_00288FB8(int);
extern int func_00115DA8(char *, const char *, ...);
extern void func_0027FAA8(int, int, unsigned long, char *, int);

void func_0033D638(char *object)
{
    char buffer[128];
    SolTextPair pair;
    char *first;
    char *second;
    char *format;
    int color;
    func_0033AF48(object + 8);
    color = *(int *)(object + 0x2A8) == 0 ? (int)0x80D0D0D0 : 0x70808080;
    pair = D_L00_001AE110;
    format = D_L00_001AE118;
    first = func_00288FB8(0x2D99);
    second = func_00288FB8(pair.ids[*(unsigned char *)0x001A7B96 != 0]);
    func_00115DA8(buffer, format, first, second);
    func_0027FAA8(0x100, 0xAA, color, buffer, -1);
}


char *func_0033D708(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033D738);



int func_0033D868(char *a0, int a1)
{
    char *p = a0 + 8;
    float *q;
    int v;

    Fb7feb893_FUN_0033AE28(p);
    q = *(float **)(a0 + 684);
    Fb7feb893_FUN_0033B018(p, q[0], q[1]);
    if (a1 & 0x1000) {
        Fb7feb893_FUN_002E5E58(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x4000) {
        Fb7feb893_FUN_002E5E58(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x40) {
        Fb7feb893_FUN_002E5E58(4, 0, 0);
        switch (*(int *)(a0 + 680)) {
        case 0:
            Fb7feb893_D_001A7BB2 = Fb7feb893_D_001A7BB2 == 0;
            break;
        case 1:
            Fb7feb893_D_001A7BB3 = Fb7feb893_D_001A7BB3 == 0;
            break;
        }
    }
    return (a1 >> 6) & 1;
}

INCLUDE_ASM("asm/game", func_0033D970);

char *func_0033DAD0(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033DB00);



int func_0033DC30(char *object, int flags)
{
    float *vec;

    Fb1bbe20c_FUN_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    Fb1bbe20c_FUN_0033B018(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = Fb1bbe20c_FUN_002E5E58(4, 0, 0);
        (void)discarded;
        Fb1bbe20c_D_001A7BB2 = (Fb1bbe20c_D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}


void func_0033DCC8(char *object)
{
    char buffer[128];
    SolTextPair pair;
    char *first;
    char *second;
    char *format;
    func_0033AF48(object + 8);
    pair = D_L00_001AE110;
    format = D_L00_001AE118;
    first = func_00288FB8(0x2D9A);
    second = func_00288FB8(pair.ids[*(unsigned char *)0x001A7BB2 != 0]);
    func_00115DA8(buffer, format, first, second);
    func_0027FAA8(0x100, 0xAA, 0x80F0F0F0UL, buffer, -1);
}



char *func_0033DD80(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033DDB0);



int func_0033DEE0(char *object, int flags)
{
    float *vec;

    Fb1bbe20c_FUN_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    Fb1bbe20c_FUN_0033B018(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = Fb1bbe20c_FUN_002E5E58(4, 0, 0);
        (void)discarded;
        Fb1bbe20c_D_001A7BB2 = (Fb1bbe20c_D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}

char *func_0033DF78(char *object)
{
    Ffb202470_FUN_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/game", func_0033DFA8);



int func_0033E0D8(char *object, int flags)
{
    float *vec;

    Fb1bbe20c_FUN_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    Fb1bbe20c_FUN_0033B018(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = Fb1bbe20c_FUN_002E5E58(4, 0, 0);
        (void)discarded;
        Fb1bbe20c_D_001A7BB2 = (Fb1bbe20c_D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}

void* func_0033E170(void* param_1)
{
  func_0033AB50((int)param_1 + 8);
  func_00347EA0((int)param_1 + 0x2b0);
  return param_1;
}

INCLUDE_ASM("asm/game", func_0033E1A8);



int func_0033E310(int owner)
{
    int result;
    result = func_0033E348(owner);
    return *(int *)(owner + 0x378) + result * 0x14;
}
