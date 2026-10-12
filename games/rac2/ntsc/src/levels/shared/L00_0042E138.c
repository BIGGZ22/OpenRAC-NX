#include "include_asm.h"

extern void func_0033AE18(unsigned char *owner, int value);

extern void func_0033AB50(char *p);

extern void func_0033AE28_2(char *p);

extern void func_L00_00430948_3(char *p, float a, float b);

extern void func_002E5E58_3(int a, int b, int c);

extern volatile unsigned char D_001A7B95 __attribute__((sda));

extern volatile unsigned char D_001A7B94 __attribute__((sda));

extern void func_0033AE28 (char *object);

extern void func_L00_00430948_1 (char *object, float x, float y);

extern int func_002E5E58_1 (int a, int b, int c);

extern void func_0033AE28_1(char *p);

extern void func_L00_00430948_2(char *p, float a, float b);

extern void func_002E5E58_2(int a, int b, int c);

extern volatile unsigned char D_001A7BB2_1 __attribute__((sda));

extern volatile unsigned char D_001A7BB3 __attribute__((sda));

void func_L00_0042E138(void)
{
}

void func_L00_0042E388(void)
{
}

void func_L00_0042E880(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_0042E9A0);

/* Forward owner+8 and the supplied signed word to the measured two-argument setter. */
void func_L00_0042F0A0(unsigned char *owner, int value)
{
    func_0033AE18(owner + 8, value);
}

INCLUDE_ASM("asm/levels", func_L00_0042F1D0);

INCLUDE_ASM("asm/levels", func_L00_0042F3F8);

INCLUDE_ASM("asm/levels", func_L00_0042F850);

INCLUDE_ASM("asm/levels", func_L00_0042FE48);

INCLUDE_ASM("asm/levels", func_L00_00430500);

/* Store the two scalar floating arguments through the supplied pointer cell; original game role is unknown. */
void func_L00_00430948(float first, float second, float **cell)
{
    (*cell)[0] = first;
    (*cell)[1] = second;
}

INCLUDE_ASM("asm/levels", func_L00_004309E0);

INCLUDE_ASM("asm/levels", func_L00_00430A90);

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00431668(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00431698);

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00431AA8(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00431AD8);

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00431D60(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00431D90);

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_004322E8(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00432318);

/* makes two float-binding calls, then toggles one of two global flag bytes selected by an object field and returns bit 6 of the flag word */
int func_L00_00432448(char *a0, int a1)
{
    char *p = a0 + 8;
    float *q;
    int v;

    func_0033AE28_2(p);
    q = *(float **)(a0 + 684);
    func_L00_00430948_3(p, q[0], q[1]);
    if (a1 & 0x1000) {
        func_002E5E58_3(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x4000) {
        func_002E5E58_3(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x40) {
        func_002E5E58_3(4, 0, 0);
        switch (*(int *)(a0 + 680)) {
        case 0:
            D_001A7B95 = D_001A7B95 == 0;
            break;
        case 1:
            D_001A7B94 = D_001A7B94 == 0;
            break;
        }
    }
    return (a1 >> 6) & 1;
}

INCLUDE_ASM("asm/levels", func_L00_004326F0);

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00432D70(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00432DA0);

/* resets a sub-object, applies a two-float transform, and when bit 6 of flags is set and an int field is zero calls a mode setter and toggles a global byte */
int func_L00_00432ED0 (char *object, int flags)
{
  volatile unsigned char *g = (volatile unsigned char *) 0x1A7B96;
  float *vec;
  int discarded;

  func_0033AE28 (object + 8);
  vec = *(float **) (object + 684);
  func_L00_00430948_1 (object + 8, vec[0], vec[1]);

  if ((flags & 0x40) && *(int *) (object + 680) == 0)
    {
      discarded = func_002E5E58_1 (4, 0, 0);
      (void) discarded;
      *g = (*g < 1);
    }

  return (flags >> 6) & 1;
}

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00433038(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00433068);

INCLUDE_ASM("asm/levels", func_L00_00433070);

/* makes two float-binding calls, then toggles one of two global flag bytes selected by an object field and returns bit 6 of the flag word */
int func_L00_00433198(char *a0, int a1)
{
    char *p = a0 + 8;
    float *q;
    int v;

    func_0033AE28_1(p);
    q = *(float **)(a0 + 684);
    func_L00_00430948_2(p, q[0], q[1]);
    if (a1 & 0x1000) {
        func_002E5E58_2(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x4000) {
        func_002E5E58_2(3, 0, 0);
        v = *(int *)(a0 + 680);
        *(int *)(a0 + 680) = (v + 1) % 2;
    } else if (a1 & 0x40) {
        func_002E5E58_2(4, 0, 0);
        switch (*(int *)(a0 + 680)) {
        case 0:
            D_001A7BB2_1 = D_001A7BB2_1 == 0;
            break;
        case 1:
            D_001A7BB3 = D_001A7BB3 == 0;
            break;
        }
    }
    return (a1 >> 6) & 1;
}

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_00433400(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_00433430);
