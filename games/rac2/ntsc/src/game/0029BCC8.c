#include "include_asm.h"

extern int ref_L00_0030CDA0_00;

void func_0029BCC8(void)
{
    ref_L00_0030CDA0_00 = -1;
}

INCLUDE_ASM("asm/game", func_0029BCD8);

extern char *D_001A8CFC;
extern void func_0034E4D8(void *);

void func_0029BE88(void) {
    if (D_001A8CFC != 0)
        func_0034E4D8(D_001A8CFC + 0x36F28);
}

extern int D_001A9A50 __attribute__((sda));
extern int D_001A9A54 __attribute__((sda));
extern int D_001A8C64;
extern void func_0033A6D8(void *);

void func_0029BEB8(void) {
    if (D_001A9A50 != 0 && D_001A8C64 == 0 && D_001A9A54 != 0 && D_001A8CFC != 0)
        func_0033A6D8(D_001A8CFC + 0x3F138);
}

INCLUDE_ASM("asm/game", func_0029BF08);

void func_0029C0D0(void) {
}

extern void func_0034E478(void *);

void func_0029C0D8(void) {
    if (D_001A8CFC != 0)
        func_0034E478(D_001A8CFC + 0x36F28);
}

extern void func_0034E4F8(void *);

void func_0029C108(void) {
    if (D_001A8CFC != 0)
        func_0034E4F8(D_001A8CFC + 0x36F28);
}

extern void func_00348EC0(void *);

void func_0029C138(void) {
    if (D_001A8CFC != 0)
        func_00348EC0(D_001A8CFC + 0x39140);
}

extern int func_003489E8(void *, int);

int func_0029C168(int value) {
    if (D_001A8CFC != 0)
        return func_003489E8(D_001A8CFC + 0x39140, value);
    return 0;
}

extern void func_003478F0(void *);

void func_0029C1A8(void) {
    if (D_001A8CFC != 0)
        func_003478F0(D_001A8CFC + 0x39590);
}

extern void func_003465D0(void *);

void func_0029C1D8(void) {
    if (D_001A8CFC != 0)
        func_003465D0(D_001A8CFC + 0x39A98);
}

extern int func_00346C60(void *, int);

int func_0029C208(int value) {
    if (D_001A8CFC != 0)
        return func_00346C60(D_001A8CFC + 0x39590, value);
    return 0;
}

extern int func_00345F70(void *, int, int);

int func_0029C248(int first, int second)
{
    if (D_001A8CFC != 0)
        return func_00345F70(D_001A8CFC + 0x39A98, first, second);
    return 0;
}

extern int func_00339320(void *, int);

int func_0029C290(int value) {
    if (D_001A8CFC != 0)
        return func_00339320(D_001A8CFC + 0x3B1F0, value);
    return 0;
}

extern void func_00339590(void *);

void func_0029C2D0(void) {
    if (D_001A8CFC != 0)
        func_00339590(D_001A8CFC + 0x3B1F0);
}

extern int func_00339F28(void *, int);

int func_0029C300(int value) {
    if (D_001A8CFC != 0)
        return func_00339F28(D_001A8CFC + 0x3ADE8, value);
    return 0;
}

extern void func_0033A3E0(void *);

void func_0029C340(void) {
    if (D_001A8CFC != 0)
        func_0033A3E0(D_001A8CFC + 0x3ADE8);
}

extern void func_0033B910(void *);

void func_0029C370(void) {
    if (D_001A8CFC != 0)
        func_0033B910(D_001A8CFC + 0x3B490);
}

extern int func_0033B540(void *, int);

int func_0029C3A0(int value) {
    if (D_001A8CFC != 0)
        return func_0033B540(D_001A8CFC + 0x3B490, value);
    return 0;
}

extern void func_0033F030(void *);

void func_0029C3E0(void) {
    if (D_001A8CFC != 0)
        func_0033F030(D_001A8CFC + 0x3B7C0);
}

extern int func_0033EA60(void *, int);

int func_0029C410(int value) {
    if (D_001A8CFC != 0)
        return func_0033EA60(D_001A8CFC + 0x3B7C0, value);
    return 0;
}

extern void func_00345358(void *);

void func_0029C450(void) {
    if (D_001A8CFC != 0)
        func_00345358(D_001A8CFC + 0x39F38);
}

extern int func_00344478(void *, int);

int func_0029C480(int value) {
    if (D_001A8CFC != 0)
        return func_00344478(D_001A8CFC + 0x39F38, value);
    return 0;
}

extern void func_00341C20(void *);

void func_0029C4C0(void) {
    if (D_001A8CFC != 0)
        func_00341C20(D_001A8CFC + 0x3BE08);
}

extern int func_003418C0(void *, int);

int func_0029C4F0(int value) {
    if (D_001A8CFC != 0)
        return func_003418C0(D_001A8CFC + 0x3BE08, value);
    return 0;
}

void func_0029C530(void) {
    if (D_001A8CFC != 0)
        func_00341C20(D_001A8CFC + 0x3BE08);
}

int func_0029C560(int value) {
    if (D_001A8CFC != 0)
        return func_003418C0(D_001A8CFC + 0x3BE08, value);
    return 0;
}

extern void func_003438E8(void *);

void func_0029C5A0(void) {
    if (D_001A8CFC != 0)
        func_003438E8(D_001A8CFC + 0x3C108);
}

extern int func_00343798(void *, int);

int func_0029C5D0(int value) {
    if (D_001A8CFC != 0)
        return func_00343798(D_001A8CFC + 0x3C108, value);
    return 0;
}

extern void func_0033E620(void *);

void func_0029C610(void) {
    if (D_001A8CFC != 0)
        func_0033E620(D_001A8CFC + 0x3C440);
}

extern int func_0033E5C0(void *, int);

int func_0029C640(int value) {
    if (D_001A8CFC != 0)
        return func_0033E5C0(D_001A8CFC + 0x3C440, value);
    return 0;
}

extern void func_0033D970(void *);

void func_0029C680(void) {
    if (D_001A8CFC != 0)
        func_0033D970(D_001A8CFC + 0x3CDB0);
}

extern int func_0033D868(void *, int);

int func_0029C6B0(int value) {
    if (D_001A8CFC != 0)
        return func_0033D868(D_001A8CFC + 0x3CDB0, value);
    return 0;
}

extern int func_0033C590(void *, int);

int func_0029C6F0(int value) {
    if (D_001A8CFC != 0)
        return func_0033C590(D_001A8CFC + 0x3D060, value);
    return 0;
}

extern void func_0033C738(void *);

void func_0029C730(void) {
    if (D_001A8CFC != 0)
        func_0033C738(D_001A8CFC + 0x3D060);
}

extern int func_0033CB18(void *, int);

int func_0029C760(int value) {
    if (D_001A8CFC != 0)
        return func_0033CB18(D_001A8CFC + 0x3D310, value);
    return 0;
}

extern void func_0033CC20(void *);

void func_0029C7A0(void) {
    if (D_001A8CFC != 0)
        func_0033CC20(D_001A8CFC + 0x3D310);
}

extern int func_0033D5A0(void *, int);

int func_0029C7D0(int value) {
    if (D_001A8CFC != 0)
        return func_0033D5A0(D_001A8CFC + 0x3D900, value);
    return 0;
}

extern void func_0033D638(void *);

void func_0029C810(void) {
    if (D_001A8CFC != 0)
        func_0033D638(D_001A8CFC + 0x3D900);
}

extern int func_0033BE98(void *, int);

int func_0029C840(int value) {
    if (D_001A8CFC != 0)
        return func_0033BE98(D_001A8CFC + 0x3DBB0, value);
    return 0;
}

extern void func_0033C028(void *);

void func_0029C880(void) {
    if (D_001A8CFC != 0)
        func_0033C028(D_001A8CFC + 0x3DBB0);
}

extern int func_0033C2D8(void *, int);

int func_0029C8B0(int value) {
    if (D_001A8CFC != 0)
        return func_0033C2D8(D_001A8CFC + 0x3DE60, value);
    return 0;
}

extern void func_0033C378(void *);

void func_0029C8F0(void) {
    if (D_001A8CFC != 0)
        func_0033C378(D_001A8CFC + 0x3DE60);
}

extern int func_0033E3A0(void *, int);

int func_0029C920(int value) {
    if (D_001A8CFC != 0)
        return func_0033E3A0(D_001A8CFC + 0x3EDB8, value);
    return 0;
}

extern void func_0033E428(void *);

void func_0029C960(void) {
    if (D_001A8CFC != 0)
        func_0033E428(D_001A8CFC + 0x3EDB8);
}

extern int func_0033D028(void *, int);

int func_0029C990(int value) {
    if (D_001A8CFC != 0)
        return func_0033D028(D_001A8CFC + 0x3D5C0, value);
    return 0;
}

extern void func_0033D2C0(void *);

void func_0029C9D0(void) {
    if (D_001A8CFC != 0)
        func_0033D2C0(D_001A8CFC + 0x3D5C0);
}

extern int func_0033DC30(void *, int);

int func_0029CA00(int value) {
    if (D_001A8CFC != 0)
        return func_0033DC30(D_001A8CFC + 0x3E5A8, value);
    return 0;
}

extern void func_0033DCC8(void *);

void func_0029CA40(void) {
    if (D_001A8CFC != 0)
        func_0033DCC8(D_001A8CFC + 0x3E5A8);
}

extern int func_003424C0(void *, int);

int func_0029CA70(int value) {
    if (D_001A8CFC != 0)
        return func_003424C0(D_001A8CFC + 0x3E1D8, value);
    return 0;
}

extern void func_00342758(void *);

void func_0029CAB0(void) {
    if (D_001A8CFC != 0)
        func_00342758(D_001A8CFC + 0x3E1D8);
}

extern int func_00341EE8(void *, int);

int func_0029CAE0(int value) {
    if (D_001A8CFC != 0)
        return func_00341EE8(D_001A8CFC + 0x3E430, value);
    return 0;
}

extern void func_00341F38(void *);

void func_0029CB20(void) {
    if (D_001A8CFC != 0)
        func_00341F38(D_001A8CFC + 0x3E430);
}

INCLUDE_ASM("asm/game", func_0029CB50);

extern void func_0028A2F8(void);

void func_0029CB58(void) {
    if (D_001A8CFC != 0)
        func_0028A2F8();
}

extern void func_0028A670(void);

void func_0029CB80(void) {
    if (D_001A8CFC != 0)
        func_0028A670();
}

extern void func_0034CE98(void *, int);

void func_0029CBA8(int value) {
    if (D_001A8CFC != 0)
        func_0034CE98(D_001A8CFC + 0x376C8, value);
}

extern int func_0033FB98(void *, int);

int func_0029CBE0(int value) {
    if (D_001A8CFC != 0)
        return func_0033FB98(D_001A8CFC + 0x3A388, value);
    return 0;
}

extern void func_00340250(void *);

void func_0029CC20(void) {
    if (D_001A8CFC != 0)
        func_00340250(D_001A8CFC + 0x3A388);
}

extern void func_00338718(void *, int, int, int);
void func_0029CC50(int a0, int a1, int a2) { if (D_001A8CFC != 0) func_00338718(D_001A8CFC + 0x36F28, a0, a1, a2); }

extern void func_003387A0(void *, int, int);

void func_0029CC98(int first, int second) {
    if (D_001A8CFC != 0)
        func_003387A0(D_001A8CFC + 0x36F28, first, second);
}

extern void func_00338040(void *, int, int);

void func_0029CCD8(int first, int second) {
    if (D_001A8CFC != 0)
        func_00338040(D_001A8CFC + 0x36F28, first, second);
}

void func_0029CD18(void) {
}

extern int D_001A7B90[] __attribute__((sda));
extern void func_00335DC0(int);

void func_0029CD20(void) {
    func_00335DC0(D_001A7B90[0] != 0);
}

INCLUDE_ASM("asm/game", func_0029CD48);

void func_0029CE38(void)
{
    extern int D_001A8C64;
    extern void func_0028E138(int);
    extern void func_0029BCD8(void);
    if (D_001A8CFC != 0) {
        if (D_001A8C64 == 0) {
            func_0028E138(1);
            func_0029BCD8();
        }
    }
}

INCLUDE_ASM("asm/game", func_0029CE78);



INCLUDE_ASM("asm/game", func_0029CEC8);

extern void func_003399B0(void *);

void func_0029CED8(void) {
    if (D_001A8CFC != 0)
        func_003399B0(D_001A8CFC + 0x3CAA8);
}

extern int func_003398A8(void *, int);

int func_0029CF08(int value) {
    if (D_001A8CFC != 0)
        return func_003398A8(D_001A8CFC + 0x3CAA8, value);
    return 0;
}

INCLUDE_ASM("asm/game", func_0029CF48);

INCLUDE_ASM("asm/game", func_0029CF58);

INCLUDE_ASM("asm/game", func_0029CF78);

INCLUDE_ASM("asm/game", func_0029CFB0);

ASM_FUNC("asm/game", func_0029D050);

INCLUDE_ASM("asm/game", func_0029D0E0);

INCLUDE_ASM("asm/game", func_0029D258);

INCLUDE_ASM("asm/game", func_0029D398);

ASM_FUNC("asm/game", func_0029D4D0);

ASM_FUNC("asm/game", func_0029D7A0);

INCLUDE_ASM("asm/game", func_0029D7C0);

INCLUDE_ASM("asm/game", func_0029D830);

INCLUDE_ASM("asm/game", func_0029D9D0);

INCLUDE_ASM("asm/game", func_0029DB28);


INCLUDE_ASM("asm/game", func_0029DC90);

INCLUDE_ASM("asm/game", func_0029DCD0);

int func_0029DD00(void) { if (*(unsigned char *)0x001A7BD5) { if (*(unsigned char *)0x001A7B08) return 1; } return 0; }

INCLUDE_ASM("asm/game", func_0029DD30);

INCLUDE_ASM("asm/game", func_0029DDC0);
