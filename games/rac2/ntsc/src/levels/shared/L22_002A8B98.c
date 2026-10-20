#include "include_asm.h"

extern char D_00189E20[];

extern int D_001A79F0;

extern int D_001A73A0;

extern unsigned char D_00139568[];

extern char D_L22_002645F0[];

extern char D_L22_001B1920[];

extern void func_L00_002D24B8(void);

extern void func_L22_002C9898(int mode, int flag);

extern void func_L00_003B09F8(void);

extern int func_L00_002ADEA0(int index);

/* Loads the level's tuning constants and starts the two subsystems. */
INCLUDE_ASM("asm/levels", func_L22_002A8B98);

/* Picks the shop entry the level's table names for the current progress state. */
INCLUDE_ASM("asm/levels", func_L22_002A8CE0);

INCLUDE_ASM("asm/levels", func_L22_002AA480);

INCLUDE_ASM("asm/levels", func_L22_002AA678);

INCLUDE_ASM("asm/levels", func_L22_002AAA30);

extern int D_00138320;

extern void func_L00_002AF7D0(void);

extern void func_L00_002B1F90(void);

extern void func_L00_00322EC8(void *p);

extern void func_L00_002D2E68(void);

extern void func_L00_002A6708(void);

extern void func_L00_002A53B8(void);

extern void func_0028B3D0(int a0, int a1, void *a2, void *a3, void *a4, int a5, int a6);

extern void func_L00_002F6B90(int a0);

extern void func_L22_002B7C70(void);

extern void func_002A677C(int a0);

/* the three routines the tuning table points at; their places differ per level */
extern void ref_L22_002B70F8_00(void);

extern void ref_L22_002B70F8_01(void);

extern void ref_L22_002B70F8_02(void);

typedef unsigned char u8;

typedef unsigned short u16;

typedef float f32;

/* Restarts the run: resets the racer's state block, plays the two subsystem
   start-ups, then arms the racer for the mode the caller asked for.
   The state block is reached through three block-scoped copies of its address,
   so each stretch of the body re-materialises the base instead of keeping it
   live across the calls; the three table entries are the routines the tuning
   table points at, whose places differ from level to level. */
void func_L22_002B70F8(int a0, int a1, void *a2) {
    char *s0 = D_00189E20;

    func_L00_002AF7D0();
    s0[0x22B6] = 1;
    func_L00_002B1F90();
    *(int *)(s0 + 0x2290) = *(int *)(s0 + 0xC18);
    func_L00_00322EC8(s0 + 0x18E0);
    func_L00_00322EC8(s0 + 0x1A30);
    *(int *)(*(int *)(s0 + 0x2290) + 0x9C) = -1;
    if (a0 != 4 || *(u8 *)(s0 + 0x22B4) != a0)
        func_L00_002D2E68();
    func_L00_002A6708();
    func_L00_002A53B8();
    {
        char *s1 = D_00189E20;

        *(u8 *)(s1 + 0x22B4) = (u8)a0;
        *(int *)(s1 + 0xC14) = (int)a2;
        *(int *)(s1 + 0x2290) = (int)a2;
    }
    if ((a0 & 0xFF) == 1) {
        if (D_00138320 & 0x10) {
            func_0028B3D0(0x23, 0, &ref_L22_002B70F8_00, &ref_L22_002B70F8_01,
                          &ref_L22_002B70F8_02, 0, 0);
            func_L00_002F6B90(3);
        }
    }
    func_L22_002B7C70();
    {
        char *s2 = D_00189E20;

        *(f32 *)(s2 + 0x250) = *(f32 *)(s2 + 0x258);
        *(f32 *)(s2 + 0x254) = *(f32 *)(s2 + 0x25C);
        *(u16 *)(*(int *)(s2 + 0x2290) + 0x34) |= 6;
        *(f32 *)(s2 + 0xC20) = 1.0f;
        *(f32 *)(s2 + 0xC24) = 1.0f;
        func_L00_002D24B8();
        func_002A677C(*(int *)(s2 + 0x2290));
        *(u16 *)(s2 + 0x24F2) = (u16)a1;
        switch (a0) {
        case 1:
            *(u16 *)(s2 + 0x24D4) = *(u16 *)(s2 + 0x24CC);
            *(int *)(s2 + 0x24CC) = *(int *)(s2 + 0x24D0);
            if (*(int *)(s2 + 0x24D0) >= 5)
                *(int *)(s2 + 0x24CC) = 4;
            break;
        case 2:
            *(int *)(s2 + 0x1874) = 0x4B0;
            break;
        }
    }
    func_L22_002C9898(a1, 2);
}

INCLUDE_ASM("asm/levels", func_L22_002B7328);

INCLUDE_ASM("asm/levels", func_L22_002B7C70);

INCLUDE_ASM("asm/levels", func_L22_002B8E10);

INCLUDE_ASM("asm/levels", func_L22_002C8610);

INCLUDE_ASM("asm/levels", func_L22_002C9898);

extern unsigned char D_0018C0D4;

extern void func_L22_002C9898(int a0, int a1);

/* plays one of three resident messages, picked by the type byte; the second argument
   is 1 or 2 depending on the caller's flag */
void func_L22_002CE468(int a0) {
    unsigned char type = D_0018C0D4;
    int mode = 1;

    if (a0 != 0)
        mode = 2;
    switch (type) {
    case 0:
        func_L22_002C9898(0, mode);
        break;
    case 1:
        func_L22_002C9898(0x3F, mode);
        break;
    case 2:
        func_L22_002C9898(0x56, mode);
        break;
    }
}
