#include "include_asm.h"

extern int D_L05_001B1FC8;

extern int D_L05_001B1FD0;

extern int D_L05_001B1FD4;

extern int D_L05_001B1FCC;

extern int D_L05_001B1FF8;

extern int D_L05_001B1FFC;

extern int D_L05_001A8F00;

extern int D_L05_001A8F08;

extern unsigned char D_0018C0D4[];

extern int D_00138320[];

extern short D_L05_001B225C;

extern short D_L05_001B225E;

extern short D_L05_001B2260;

extern short D_L05_001B2262;

extern short D_L05_001B2264;

#ifndef RAC2_T_F32_F2144BF3D
#define RAC2_T_F32_F2144BF3D
typedef float f32_F2144bf3d;
#endif

#ifndef RAC2_T_S32_F2144BF3D
#define RAC2_T_S32_F2144BF3D
typedef int s32_F2144bf3d;
#endif

/* Resident record reached through a fixed core symbol: the retail body keeps
   the base in a register and addresses every field by displacement. */
#ifndef RAC2_T_RESIDENT138180_F2144BF3D
#define RAC2_T_RESIDENT138180_F2144BF3D
typedef struct {
    char  pad0[0x100];
    f32_F2144bf3d   f100;                 /* 0x100 */
    f32_F2144bf3d   f104;                 /* 0x104 */
    f32_F2144bf3d   f108;                 /* 0x108 */
    f32_F2144bf3d   f10C;                 /* 0x10C */
    char  pad1[0x1A0 - 0x110];
    s32_F2144bf3d   i1A0;                 /* 0x1A0 */
} Resident138180_F2144bf3d;
#endif

extern char D_00138180[];

extern s32_F2144bf3d D_0018D198[];

extern s32_F2144bf3d D_L06_001AD3A0 __attribute__((sda));

extern s32_F2144bf3d D_001A7390;

extern s32_F2144bf3d D_001A79F0;

extern s32_F2144bf3d D_L06_001B2348;

extern s32_F2144bf3d D_L06_001A8FC0;

extern s32_F2144bf3d D_L06_001B2900;

extern s32_F2144bf3d D_L06_001AA0F0;

void func_002F5220(void)
{
    unsigned char mode;

    D_L05_001B1FC8 = 0;
    mode = D_0018C0D4[0];
    D_L05_001B1FD0 = 0;
    D_L05_001B1FD4 = 0;
    D_L05_001B1FCC = 0;
    D_L05_001B1FF8 = 0;
    D_L05_001B1FFC = 0;

    if (mode == 1)
    {
        if ((D_00138320[0] & 0x10) == 0)
        {
            if (D_L05_001A8F00 != 4 && D_L05_001A8F08 != 4)
            {
                D_L05_001B225C = 0;
                D_L05_001B225E = 0;
                D_L05_001B2260 = 0;
                D_L05_001B2262 = 0;
                D_L05_001B2264 = 0;
            }
        }
    }
}

void func_002F52B8(void)
{
    Resident138180_F2144bf3d *p = (Resident138180_F2144bf3d *)D_00138180;

    if (p->i1A0 != 0) goto mismatch;
    if (p->f108 != 0.0f) goto mismatch;
    if (p->f10C != 0.0f) goto mismatch;
    if (p->f100 != 0.0f) goto mismatch;
    if (p->f104 == 0.0f) goto match;
mismatch:
    D_L06_001AD3A0 = 0;
    goto gated;
match:
    D_L06_001AD3A0 = D_L06_001AD3A0 + 1;
gated:
    D_L06_001B2348 = D_L06_001B2348 + 1;
    D_001A7390 = D_001A7390 + 1;

    if (D_L06_001A8FC0 != 0) {
        if (D_L06_001AD3A0 < 900) {
            D_0018D198[D_001A79F0] = D_0018D198[D_001A79F0] + 1;
        }
    }

    if (D_L06_001B2900 != D_001A7390 - 1) {
        D_L06_001AA0F0 = -1;
    }
}

INCLUDE_ASM("asm/game", func_002F53C0);

INCLUDE_ASM("asm/game", func_002F53C8);

INCLUDE_ASM("asm/game", func_002F57A8);

INCLUDE_ASM("asm/game", func_002F5AE8);

INCLUDE_ASM("asm/game", func_002F5AF8);

extern int D_001A8C80[] __attribute__((sda));
extern int D_001A8C84 __attribute__((sda));

void func_002F5CB8(int first, int second) {
    D_001A8C80[0] = first;
    D_001A8C84 = second;
}

INCLUDE_ASM("asm/game", func_002F5CC8);

INCLUDE_ASM("asm/game", func_002F5D10);

INCLUDE_ASM("asm/game", func_002F5D40);

INCLUDE_ASM("asm/game", func_002F5E20);

INCLUDE_ASM("asm/game", func_002F5EF8);

INCLUDE_ASM("asm/game", func_002F5FB8);

extern void func_288CB0(int, int);
void func_002F6458(void) { char *base = D_00138180 + 0x1260; int index = *(int *)(base + 0x148); if (index != -1) { if (*(short *)(base + index * 0xA0 + 0x18) >= 0) *(int *)(base + 0x17C) = 1; } func_288CB0(0, 0); }

INCLUDE_ASM("asm/game", func_002F64A8);

INCLUDE_ASM("asm/game", func_002F6510);

INCLUDE_ASM("asm/game", func_002F6608);

INCLUDE_ASM("asm/game", func_002F71B8);

INCLUDE_ASM("asm/game", func_002F7320);

INCLUDE_ASM("asm/game", func_002F73A8);

INCLUDE_ASM("asm/game", func_002F7738);

INCLUDE_ASM("asm/game", func_002F8178);

INCLUDE_ASM("asm/game", func_002F8768);

typedef struct { unsigned char pad0[0x3C]; char *destination; unsigned char pad1[0x24]; int clear; } Func_002F88D8State;
extern Func_002F88D8State ref_L00_002F88D8_00;
extern void func_00115DA8(char *, const char *, ...);
extern const char D_001AD2B8[];

void func_002F88D8(void *arg) { func_00115DA8(ref_L00_002F88D8_00.destination, D_001AD2B8, arg); ref_L00_002F88D8_00.clear = 0; }

INCLUDE_ASM("asm/game", func_002F8918);

INCLUDE_ASM("asm/game", func_002F9198);

INCLUDE_ASM("asm/game", func_002F93A8);

INCLUDE_ASM("asm/game", func_002F93B0);

extern void func_002F93B0(void);

void func_002F9778(void)
{
    func_002F93B0();
}

INCLUDE_ASM("asm/game", func_002F9798);

INCLUDE_ASM("asm/game", func_002F97A0);
