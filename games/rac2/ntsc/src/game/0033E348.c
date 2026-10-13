#include "include_asm.h"

extern int func_0033E348(int);

extern int func_00348128(unsigned char *);

/* Qwen draft (branch qwen/autonomy, 4419d477), checked by tools/check.py */
void func_0033AF48(int);

extern void func_0033AB50_1(void *);

extern void func_L00_0042BFF8(void *);

extern void func_L00_0042C290(void *);

extern void func_L00_0042BAD0(void *);

int func_0033E348(int owner)
{
    return func_00348128((unsigned char *)(owner + 0x2b0));
}

void func_0033E368(unsigned char *owner, float *records)
{
    extern void func_00348068(unsigned char *, float *);
    func_00348068(owner + 0x2B0, records);
    *(float **)(owner + 0x378) = records;
}

INCLUDE_ASM("asm/game", func_0033E3A0);

INCLUDE_ASM("asm/game", func_0033E428);

extern void func_0033AB50(void *);
extern void *func_00336968(void *);

char *func_0033E480(char *object) {
    func_0033AB50(object + 8);
    func_00336968(object + 0x2B0);
    return object;
}

INCLUDE_ASM("asm/game", func_0033E4B8);

INCLUDE_ASM("asm/game", func_0033E4C0);

extern void func_0033AE28(char *p);
extern void func_0033B018(char *p, float x, float y);

int func_0033E5C0(char *object, int flags)
{
    char *p = object + 8;
    float *v;
    func_0033AE28(p);
    v = *(float **)(object + 0x2A8);
    func_0033B018(p, v[0], v[1]);
    return (flags >> 6) & 1;
}

void func_0033E620(int param_1) {
  func_0033AF48(param_1 + 8);
}

/* Calls twelve sub-object handlers in sequence, each with the object base plus a fixed byte offset (8, 684, 744, 816, 888, 960, 1040, 1112, 1184, 1256, 1328, 1400), then returns the object pointer. */
char *func_0033E640(char *object)
{
    func_0033AB50_1(object + 8);
    func_L00_0042BFF8(object + 684);
    func_L00_0042C290(object + 744);
    func_L00_0042C290(object + 816);
    func_L00_0042C290(object + 888);
    func_L00_0042BAD0(object + 960);
    func_L00_0042C290(object + 1040);
    func_L00_0042C290(object + 1112);
    func_L00_0042C290(object + 1184);
    func_L00_0042C290(object + 1256);
    func_L00_0042C290(object + 1328);
    func_L00_0042C290(object + 1400);
    return object;
}

INCLUDE_ASM("asm/game", func_0033E6C8);

INCLUDE_ASM("asm/game", func_0033E6D0);

INCLUDE_ASM("asm/game", func_0033EA60);
