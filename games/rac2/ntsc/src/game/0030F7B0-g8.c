#include "include_asm.h"

extern char D_001B5340[];
extern int func_002A77E0(int);
extern float D_001AF1A4 __attribute__((sda));
extern float D_001AF1A8 __attribute__((sda));
extern unsigned char D_001AF1AC __attribute__((sda));
extern void *D_001AF1B0 __attribute__((sda));
extern void *D_001AF1B4 __attribute__((sda));
extern void *D_001AF1B8 __attribute__((sda));
extern float D_001AF1BC __attribute__((sda));
extern float D_001AF1C0 __attribute__((sda));
extern char D_0018A0E0[];
extern void func_00282CF0(void *, void *, void *);
extern float func_002A7878(float, float);
extern void func_00282D30(void *, void *, float);
extern float func_002A7940(void);
extern void func_002ACEC0(void *, void *, void *, float);
extern void func_002C3150(void *, void *, void *, int, int, int, float);

void func_003130F0(unsigned char *object)
{
    char first[16];
    char second[16];
    char third[16];
    unsigned char *state;
    char *basis;
    void *effect;
    unsigned char kind;
    unsigned char random;

    state = *(unsigned char **)(object + 0x68);
    func_00282CF0(first, D_001B5340, object + 0x10);
    basis = D_0018A0E0;
    func_00282D30(second, basis,
                  func_002A7878(D_001AF1A4 * 0.016666668f,
                               D_001AF1A8 * 0.016666668f));
    func_002ACEC0(third, second, first, func_002A7940());

    effect = state[0x57] != 0 ? D_001AF1B8 : D_001AF1B0;
    kind = D_001AF1AC;
    random = (unsigned char)func_002A77E0(255);
    func_002C3150(object + 0x10, third, effect, kind, random, 1, D_001AF1BC);

    kind = D_001AF1AC;
    random = (unsigned char)func_002A77E0(255);
    func_002C3150(object + 0x10, third, D_001AF1B4, kind, random, 1, D_001AF1C0);
}

extern char *D_001AF880 __attribute__((sda));

void func_00317390(void)
{
    char *p = D_001AF880;
    if (p != 0) {
        char *q = *(char **)(p + 0x68);
        *(int *)(q + 0x28) = 1;
    }
}
