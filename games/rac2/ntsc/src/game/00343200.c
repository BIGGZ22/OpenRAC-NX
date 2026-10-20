#include "include_asm.h"

typedef unsigned char u8;

extern void func_L00_0042BAD0_3(u8 *);

extern void func_L00_0042BFF8_2(u8 *);

extern void func_L00_0042C290_3(u8 *);

extern void func_L00_004380E0_1(u8 *);

/* Measured partial C views; original types and object ownership are unknown. */
typedef unsigned char QwenRecovery_a2544d80ec96_AbiByte22;

extern void func_00342F70(QwenRecovery_a2544d80ec96_AbiByte22 *owner, const QwenRecovery_a2544d80ec96_AbiByte22 *table);

extern int func_00342BC0(unsigned char *owner);

extern void func_00343058(unsigned char *owner);

typedef int s32;

typedef unsigned int u32;

/* initialises eight in-place sub-records of one object by four routines, then returns the object pointer */
u8 *func_00343200(u8 *a0) {
    func_L00_0042BAD0_3(a0);
    func_L00_0042BAD0_3(a0 + 76);
    func_L00_0042BAD0_3(a0 + 152);
    func_L00_0042BFF8_2(a0 + 228);
    func_L00_0042C290_3(a0 + 432);
    func_L00_0042C290_3(a0 + 504);
    func_L00_0042C290_3(a0 + 576);
    func_L00_004380E0_1(a0 + 664);
    return a0;
}

extern int func_00336E40(void *);
extern int *func_00336DA8(int, int);
extern char D_001ADC10[], D_001ADC18[], D_001ADC20[], D_001ADC30[], D_001ADC88[], D_001AE0C0[];
/* sda in this -G0 unit gives the one-instruction (macro) load of D_001A8CFC that retail has; a plain
   declaration splits it into lui/lw with two registers and shifts the register allocation */
extern char *D_001A8CFC[] __attribute__((sda));
extern void func_003361E0(void *, void *, void *);
extern void func_003369A8(void *, void *, void *);
extern void func_00336710(void *, void *, void *);
extern int *func_00335E20(void *);
extern void func_00336390(void *, void *, int);
extern void func_00336A38(void *, void *);
extern const char *func_00288FB8(int);
extern void func_00335F88(void *, float, float, float, float);
extern void func_003428E0(void *, void *);
extern void func_00343028(void *, float, float);
extern void func_00343038(void *, float);
extern void func_00343798(void *, int);
extern void func_003435B0(u8 *, u32);

/* constructor: stores the owner at +0x28C, allocates the 16-byte position at +0x288 and initialises the
   sub-objects at +0x0, +0x4C, +0x98, +0xE4, +0x1B0, +0x1F8, +0x240 and +0x298 */
void func_00343260(char *self, void *argument)
{
    *(void **)(self + 0x28C) = argument;
    if (argument) {
        int *allocation = func_00336DA8(16, func_00336E40(argument));
        *(int **)(self + 0x288) = allocation;
        allocation[1] = 0;
        allocation[2] = 0;
        allocation[3] = 0;
        allocation[0] = 0;
    }
    *(int *)(self + 0x290) = 1;
    **(float **)(self + 0x288) = 251.0f;
    (*(float **)(self + 0x288))[1] = 210.0f;
    func_003361E0(self, D_001ADC10, argument);
    func_003361E0(self + 0x4C, D_001ADC18, argument);
    func_003361E0(self + 0x98, D_001ADC20, argument);
    func_003369A8(self + 0x1B0, D_001ADC30, argument);
    func_003369A8(self + 0x1F8, D_001ADC88, argument);
    func_003369A8(self + 0x240, D_001ADC88, argument);
    func_00336710(self + 0xE4, D_001AE0C0, argument);
    *func_00335E20(self) = 0x60442D00;
    *func_00335E20(self + 0x4C) = 0x60241700;
    *func_00335E20(self + 0x98) = 0x55F0C070;
    *func_00335E20(self + 0x1B0) = 0x80F0F0F0;
    *func_00335E20(self + 0x240) = 0x80F0F0F0;
    *func_00335E20(self + 0x1F8) = 0x80F0F0F0;
    *func_00335E20(self + 0xE4) = 0x80F0F0F0;
    func_00336390(self, D_001A8CFC[0] + 0x8710, 0x99);
    func_00336390(self + 0x4C, D_001A8CFC[0] + 0x8710, 0x9A);
    func_00336390(self + 0x98, D_001A8CFC[0] + 0x8710, 0x9B);
    func_00336A38(self + 0x1B0, self + 0x120);
    func_00336A38(self + 0x1F8, self + 0x160);
    func_00336A38(self + 0x240, (void *)func_00288FB8(0x2D4F));
    func_003435B0((u8 *)self, 0);
    func_00335F88(self + 0xE4, 32.0f, 32.0f, 0.0f, 0.0f);
    func_003428E0(self + 0x298, argument);
    func_00343028(self + 0x298, 0.0f, 40.0f);
    func_00343038(self + 0x298, 32.0f);
    *(int *)(self + 0x330) = 0;
    func_00343798(self, 0);
}

void func_00343538(QwenRecovery_a2544d80ec96_AbiByte22 *p1, const QwenRecovery_a2544d80ec96_AbiByte22 *table)
{
    func_00342F70(p1 + 0x298, table);
}

INCLUDE_ASM("asm/game", func_00343558);

int func_00343560(unsigned char *owner)
{
    return func_00342BC0(owner + 0x298);
}

void func_00343580(unsigned char *owner)
{
    func_00343058(owner + 0x298);
}

/* Measured target: byte-identical across all 27 levels. */
void func_003435A0(u8 *a0, s32 a1) { *(s32 *)(a0 + 0x330) = a1; }

/* Measured target: byte-identical across all 27 levels. */
s32 func_003435A8(u8 *a0) { return *(s32 *)(a0 + 0x330); }

void func_003435B0(u8 *a0, u32 a1) {
    if (a1 < 4) *(u32 *)(a0 + 0x294) = a1;
    *(s32 *)(a0 + 0x1a0) = 0;
}



/* func_00343638 is matched in 00343200-g8.c (-G8 small data). */
