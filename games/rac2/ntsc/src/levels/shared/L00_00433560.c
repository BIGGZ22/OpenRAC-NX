#include "include_asm.h"

extern void func_0033AE28(char *p);

extern void func_L00_00430948_1(char *p, float x, float y);

extern int func_002E5E58_1(int a, int b, int c);

extern volatile unsigned char D_001A7BB2 __attribute__((sda));

extern void func_0033AB50(char *p);

void* func_L00_00433AA0(void* param_1);

extern void func_0033AB50_1(int);

extern unsigned long long func_00347EA0_1(unsigned long long);

extern int func_00342BC0(unsigned char *owner);

extern void func_00343058(unsigned char *owner);

extern void func_L00_0042BAD0(char *p);

extern void func_L00_0042C290(char *p);

extern void func_L00_0042BFF8(char *p);

/* Calls two helpers on object-plus-8 using the pointer at plus-684; when flag bit 6 is set and the plus-680 word is zero, calls a third helper and normalizes the measured byte cell to 0 or 1; returns flag bit 6. */
int func_L00_00433560(char *object, int flags)
{
    float *vec;

    func_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    func_L00_00430948_1(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = func_002E5E58_1(4, 0, 0);
        (void)discarded;
        D_001A7BB2 = (D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_004336B0(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_004336E0);

INCLUDE_ASM("asm/levels", func_L00_004336E8);

/* Calls two helpers on object-plus-8 using the pointer at plus-684; when flag bit 6 is set and the plus-680 word is zero, calls a third helper and normalizes the measured byte cell to 0 or 1; returns flag bit 6. */
int func_L00_00433810(char *object, int flags)
{
    float *vec;

    func_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    func_L00_00430948_1(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = func_002E5E58_1(4, 0, 0);
        (void)discarded;
        D_001A7BB2 = (D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}

/* Calls a helper with a pointer to the second 16-byte field of the record and returns the record pointer. */
char *func_L00_004338A8(char *object)
{
    func_0033AB50(object + 8);
    return object;
}

INCLUDE_ASM("asm/levels", func_L00_004338D8);

INCLUDE_ASM("asm/levels", func_L00_004338E0);

/* Calls two helpers on object-plus-8 using the pointer at plus-684; when flag bit 6 is set and the plus-680 word is zero, calls a third helper and normalizes the measured byte cell to 0 or 1; returns flag bit 6. */
int func_L00_00433A08(char *object, int flags)
{
    float *vec;

    func_0033AE28(object + 8);
    vec = *(float **)(object + 684);
    func_L00_00430948_1(object + 8, vec[0], vec[1]);

    if ((flags & 0x40) && *(int *)(object + 680) == 0) {
        int discarded = func_002E5E58_1(4, 0, 0);
        (void)discarded;
        D_001A7BB2 = (D_001A7BB2 < 1);
    }

    return (flags >> 6) & 1;
}

/* Initialize the owner subviews at 8 and 0x 2B 0, then return the supplied owner pointer. */
void* func_L00_00433AA0(void* param_1)
{
  func_0033AB50_1((int)param_1 + 8);
  func_00347EA0_1((int)param_1 + 0x2b0);
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_00433AD8);

INCLUDE_ASM("asm/levels", func_L00_00433DB0);

INCLUDE_ASM("asm/levels", func_L00_00433DE8);

extern int func_0033AF48(char *p);

int func_L00_00433F50(char *p)
{
    return func_0033AF48(p + 8);
}

INCLUDE_ASM("asm/levels", func_L00_00433FF8);

INCLUDE_ASM("asm/levels", func_L00_00434B40);

void func_L00_004370F8(void)
{
}

INCLUDE_ASM("asm/levels", func_L00_00437600);

INCLUDE_ASM("asm/levels", func_L00_00437780);

extern int func_00348128(char *p);

int func_L00_004377C0(char *p)
{
    return func_00348128(p + 16);
}

void func_L00_00437D50(void)
{
}

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_004380E0(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_004380E8);

INCLUDE_ASM("asm/levels", func_L00_00438950);

/* Store the two scalar floating arguments through the supplied pointer cell; original game role is unknown. */
void func_L00_00438970(float first, float second, float **cell)
{
    (*cell)[0] = first;
    (*cell)[1] = second;
}

extern int func_00342F70(char *p);

int func_L00_00438E68(char *p)
{
    return func_00342F70(p + 664);
}

INCLUDE_ASM("asm/levels", func_L00_00438E88);

/* Return the measured signed-word getter result for the owner subview at 0x 298. */
int func_L00_00438E90(unsigned char *owner)
{
    return func_00342BC0(owner + 0x298);
}

/* Invoke the measured cleanup helper on the owner subview at 0x 298. */
void func_L00_00438EB0(unsigned char *owner)
{
    func_00343058(owner + 0x298);
}

INCLUDE_ASM("asm/levels", func_L00_00439210);

/* Calls a fixed chain of sub-object initialisers at 76-byte steps and returns the base pointer. */
char *func_L00_004392D0(char *p)
{
    func_L00_0042BAD0(p);
    func_L00_0042BAD0(p + 76);
    func_L00_0042BAD0(p + 152);
    func_L00_0042BAD0(p + 228);
    func_L00_0042BAD0(p + 304);
    func_L00_0042BAD0(p + 380);
    func_L00_0042C290(p + 456);
    func_L00_0042C290(p + 528);
    func_L00_0042BFF8(p + 600);
    func_L00_0042C290(p + 664);
    func_L00_0042C290(p + 736);
    func_L00_0042C290(p + 808);
    func_L00_0042C290(p + 880);
    func_L00_0042C290(p + 952);
    return p;
}

/* Calls a fixed chain of sub-object initialisers at 76-byte steps and returns the base pointer. */
char *func_L00_0043AD30(char *p)
{
    func_L00_0042BAD0(p);
    func_L00_0042BAD0(p + 76);
    func_L00_0042BAD0(p + 152);
    func_L00_0042BAD0(p + 228);
    func_L00_0042BAD0(p + 304);
    func_L00_0042BAD0(p + 380);
    func_L00_0042C290(p + 456);
    func_L00_0042C290(p + 528);
    func_L00_0042BFF8(p + 600);
    func_L00_0042C290(p + 664);
    func_L00_0042C290(p + 736);
    func_L00_0042C290(p + 808);
    func_L00_0042C290(p + 880);
    func_L00_0042C290(p + 952);
    return p;
}

INCLUDE_ASM("asm/levels", func_L00_0043D990);

/* Store the supplied unsigned 32-bit word at owner byte offset 0xB 8. */
void func_L00_0043DA18(int param_1, unsigned int param_2)
{
  *(unsigned int *)(param_1 + 0xb8) = param_2;
  return;
}

INCLUDE_ASM("asm/levels", func_L00_0043DE98);

INCLUDE_ASM("asm/levels", func_L00_0043E310);

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_0043EA78(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_0043EEB8);

INCLUDE_ASM("asm/levels", func_L00_0043F298);

INCLUDE_ASM("asm/levels", func_L00_0043F388);

INCLUDE_ASM("asm/levels", func_L00_0043F680);

INCLUDE_ASM("asm/levels", func_L00_0043FDD0);

void func_L00_004427C8(char *object, int value) {
    *(int *)(object + 0x157C) = value;
}

void func_L00_004427D0(char *object, int value) {
    *(int *)(object + 0x1580) = value;
}

INCLUDE_ASM("asm/levels", func_L00_00443918);

void func_L00_00443B58(void)
{
}

extern int func_00338A60(char *p);

int func_L00_00443DF8(char *p)
{
    return func_00338A60(p + 0x1D6C);
}

extern int func_00338F58(char *p);

int func_L00_00443E18(char *p)
{
    return func_00338F58(p + 0x1D6C);
}

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_00443EF8(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_00444D18);

INCLUDE_ASM("asm/levels", func_L00_004450C0);

INCLUDE_ASM("asm/levels", func_L00_004450E8);

INCLUDE_ASM("asm/levels", func_L00_004451F8);

void func_L00_00445EE8(void)
{
}

/* Return the constant signed integer one. */
/* Returns the constant integer 1. Matches FUN_0012F940 disassembly and decompilation. */
int func_L00_00445FE8(void) {
    return 1;
}

INCLUDE_ASM("asm/levels", func_L00_00447160);

extern int func_00350A78(char *p);

int func_L00_00447188(char *p)
{
    return func_00350A78(p + 0x48);
}

extern int func_00350B70(char *p);

int func_L00_004471A8(char *p)
{
    return func_00350B70(p + 0x48);
}

extern int func_00351268(char *p);

int func_L00_00447278(char *p)
{
    return func_00351268(p + 0x48);
}

INCLUDE_ASM("asm/levels", func_L00_00447298);

INCLUDE_ASM("asm/levels", func_L00_00447678);

INCLUDE_ASM("asm/levels", func_L00_004476A8);

void func_L00_00447780(void)
{
}
