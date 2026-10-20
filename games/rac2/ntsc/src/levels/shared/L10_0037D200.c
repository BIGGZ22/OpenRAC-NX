#include "include_asm.h"

typedef unsigned char u8;

typedef struct { int status; u8 reserved[36]; } GornStatusRecord156;

extern GornStatusRecord156 D_L10_002A3C70[];

INCLUDE_ASM("asm/levels", func_L10_0037D200);

/* Report whether any configured status in the selected eleven records is nonzero */
int func_L10_0037F510(void)
{
    if (D_L10_002A3C70[2].status != 0 ||
        D_L10_002A3C70[3].status != 0 ||
        D_L10_002A3C70[4].status != 0 ||
        D_L10_002A3C70[5].status != 0 ||
        D_L10_002A3C70[8].status != 0 ||
        D_L10_002A3C70[9].status != 0 ||
        D_L10_002A3C70[10].status != 0 ||
        D_L10_002A3C70[11].status != 0 ||
        D_L10_002A3C70[12].status != 0 ||
        D_L10_002A3C70[13].status != 0 ||
        D_L10_002A3C70[14].status != 0)
        return 1;
    return 0;
}

INCLUDE_ASM("asm/levels", func_L10_0039D5E0);

extern void func_L10_0039D760(void);
extern void func_L00_002E6EF8(void *, void *);
extern u8 D_L10_0039D9C0[];

/* Refreshes and then hands the constructor at 0x0039D9C0 to the callback list. */
void func_L10_0039D728(void *a0) {
    func_L10_0039D760();
    func_L00_002E6EF8(D_L10_0039D9C0, a0);
}

INCLUDE_ASM("asm/levels", func_L10_0039D760);

extern int D_L10_001AEEC0 __attribute__((sda));

extern int func_00282948(void *);

extern void func_0029F338(void *);

extern char D_L10_00234600[];

extern void func_L10_0039DA48(void *, int, int, int, float, float, float, int, int);

/* Ticks the level's spawn counter down to zero, else spends the record through the effect
   constructor at 0x0039DA48 with the record's own vectors. */
void func_L10_0039D9C0(void *a0) {
    char *s0 = *(char **)((char *)a0 + 0x68);

    if (func_00282948(s0 + 0x10) == 0) {
        func_L10_0039DA48(D_L10_00234600, *(int *)(s0 + 0x1C), *(int *)(s0 + 0x30), 8,
                          *(float *)(s0 + 0x24), *(float *)(s0 + 0x28),
                          *(float *)(s0 + 0x20), *(int *)(s0 + 0x14), *(int *)(s0 + 0x18));
    } else {
        D_L10_001AEEC0 = D_L10_001AEEC0 - 1;
        if (D_L10_001AEEC0 < 0)
            D_L10_001AEEC0 = 0;
        func_0029F338(a0);
    }
}

INCLUDE_ASM("asm/levels", func_L10_0039DA48);

INCLUDE_ASM("asm/levels", func_L10_00418D88);

INCLUDE_ASM("asm/levels", func_L10_004190F0);

extern int func_002ED688(void *);

extern int func_002A77E0(int);

extern int func_002E5CF8(int, int, char *);

/* The engine's effect call: four floats in $f12-$f15, the object in $a0 and seven
   words in $a1-$a3, $t0-$t2. The fourth float sits after the object arguments
   because the retail code sets the registers up in that order. */
extern void func_L05_00379FA0(float, float, float, void *, int, int, int, float, int, int, int);

/* Kills the object: the type at 0x20 picks the effect, and the hit state at 0x268
   ends up at 0x19. */
void func_L10_004191E8(void *a0)
{
    void *s0 = a0;
    void *s1 = *(void **)((char *)s0 + 0x68);
    void *v0 = func_002ED688(s0);

    if (v0 == 0)
        return;
    if (*(int *)((char *)v0 + 0x120) == 0)
        return;
    if (*(int *)((char *)s1 + 0x268) != 0)
        return;
    if (*(unsigned char *)((char *)s0 + 0x20) == 2) {
        func_L05_00379FA0(0.9f, 13.0f, 4.5f, s0, 0x60000080, 0, 1, 1.0f, 0, -1, 0);
    } else if (*(unsigned char *)((char *)s0 + 0x20) == 1) {
        if (func_002A77E0(2) != 0)
            func_L05_00379FA0(0.6f, 5.0f, 1.0f, s0, 0x60800000, 1, 0, 1.0f, 0, -1, 0);
        else
            func_L05_00379FA0(0.6f, 5.0f, 1.0f, s0, 0x60800080, 1, 0, 1.0f, 0, -1, 0);
    }
    if (func_002A77E0(3) == 0)
        func_002E5CF8(0, 0, s0);
    *(int *)((char *)s1 + 0x268) = 0x19;
}

INCLUDE_ASM("asm/levels", func_L10_00419348);

INCLUDE_ASM("asm/levels", func_L10_004235A0);

INCLUDE_ASM("asm/levels", func_L10_00423698);
