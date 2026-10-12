#include "include_asm.h"

typedef unsigned char u8;

typedef unsigned int u32;

typedef int s32;

/* Three measured byte-state cases and width-specific field views.
   The correspondence name UpdateMoby_2426 does not establish an original
   class, enumeration, allocation or field-name declaration. */
extern void func_002A7750(u8 *, u8, s32);

extern u32 func_002B38B8(u8 *);

typedef float f32;

typedef struct {
    u32 field00;
    u32 field04;
    f32 field08;
    f32 field0C;
} NativeCallbackTimer;

extern NativeCallbackTimer D_L04_001B98E0;

extern f32 D_L04_001B1C88 __attribute__((sda));

INCLUDE_ASM("asm/levels", func_L03_003C5B28);

INCLUDE_ASM("asm/levels", func_L03_003C5CE8);

INCLUDE_ASM("asm/levels", func_L03_003C5DC8);

INCLUDE_ASM("asm/levels", func_L03_003C5EE8);

INCLUDE_ASM("asm/levels", func_L03_003C9FB8);

int func_L03_003CA138(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003CA148);

INCLUDE_ASM("asm/levels", func_L03_003CA220);

INCLUDE_ASM("asm/levels", func_L03_003CA230);

int func_L03_003CA280(unsigned char *p) { return p[0x20] == 1; }

INCLUDE_ASM("asm/levels", func_L03_003CA290);

INCLUDE_ASM("asm/levels", func_L03_003CA4B0);

void func_L03_003CA9F8(u8 *entity) {
    switch (entity[0x20]) {
    case 0:
        func_002A7750(entity, 1, -1);
        *(u32 *)(entity + 0x98) = 0;
        *(unsigned short *)(entity + 0x34) =
            (*(unsigned short *)(entity + 0x34) | 0x41) & 0xEFFF;
        break;
    case 1:
        if (func_002B38B8(entity) != 0) {
            func_002A7750(entity, 2, -1);
            *(u32 *)(entity + 0x98) =
                *(u32 *)(*(u8 **)(entity + 0x24) + 0x10);
        }
        break;
    case 2:
        if (func_002B38B8(entity) == 0) {
            func_002A7750(entity, 1, -1);
            *(u32 *)(entity + 0x98) = 0;
        }
        break;
    }
}

INCLUDE_ASM("asm/levels", func_L03_003CAAC8);

INCLUDE_ASM("asm/levels", func_L03_003CAAF0);

INCLUDE_ASM("asm/levels", func_L03_003CAB80);

INCLUDE_ASM("asm/levels", func_L03_003CC1D8);

INCLUDE_ASM("asm/levels", func_L03_003CC238);

INCLUDE_ASM("asm/levels", func_L03_003CC478);

INCLUDE_ASM("asm/levels", func_L03_003D04E0);

INCLUDE_ASM("asm/levels", func_L03_003D09D0);

INCLUDE_ASM("asm/levels", func_L03_003D0AE0);

INCLUDE_ASM("asm/levels", func_L03_003D0FC8);

INCLUDE_ASM("asm/levels", func_L03_003D9578);

INCLUDE_ASM("asm/levels", func_L03_003D9820);

INCLUDE_ASM("asm/levels", func_L03_003E21B0);

INCLUDE_ASM("asm/levels", func_L03_003E26C8);

INCLUDE_ASM("asm/levels", func_L03_003E2968);

INCLUDE_ASM("asm/levels", func_L03_003E2C08);

INCLUDE_ASM("asm/levels", func_L03_003E2D98);

INCLUDE_ASM("asm/levels", func_L03_003E2EC8);

INCLUDE_ASM("asm/levels", func_L03_003E3018);

INCLUDE_ASM("asm/levels", func_L03_003E3308);

INCLUDE_ASM("asm/levels", func_L03_003E3430);

INCLUDE_ASM("asm/levels", func_L03_003E3678);

INCLUDE_ASM("asm/levels", func_L03_003E3750);

INCLUDE_ASM("asm/levels", func_L03_003E3800);

INCLUDE_ASM("asm/levels", func_L03_003E3A00);

INCLUDE_ASM("asm/levels", func_L03_003E4148);

INCLUDE_ASM("asm/levels", func_L03_003E4588);

INCLUDE_ASM("asm/levels", func_L03_003E4680);

INCLUDE_ASM("asm/levels", func_L03_003E5070);

INCLUDE_ASM("asm/levels", func_L03_003E50E0);

INCLUDE_ASM("asm/levels", func_L03_003E5170);

INCLUDE_ASM("asm/levels", func_L03_003E5218);

INCLUDE_ASM("asm/levels", func_L03_00408068);

INCLUDE_ASM("asm/levels", func_L03_00409248);

INCLUDE_ASM("asm/levels", func_L03_00409AB0);

INCLUDE_ASM("asm/levels", func_L03_00409C38);

INCLUDE_ASM("asm/levels", func_L03_00409D28);

INCLUDE_ASM("asm/levels", func_L03_00419578);

INCLUDE_ASM("asm/levels", func_L03_004197A8);

INCLUDE_ASM("asm/levels", func_L03_004197C8);

INCLUDE_ASM("asm/levels", func_L03_00419818);

INCLUDE_ASM("asm/levels", func_L03_00419850);

INCLUDE_ASM("asm/levels", func_L03_004199F0);

INCLUDE_ASM("asm/levels", func_L03_00419A60);

INCLUDE_ASM("asm/levels", func_L03_00419B10);

INCLUDE_ASM("asm/levels", func_L03_00419B58);

INCLUDE_ASM("asm/levels", func_L03_00419B90);

INCLUDE_ASM("asm/levels", func_L03_00419BD8);

INCLUDE_ASM("asm/levels", func_L03_00419C00);

INCLUDE_ASM("asm/levels", func_L03_00419C58);

INCLUDE_ASM("asm/levels", func_L03_0041B848);

INCLUDE_ASM("asm/levels", func_L03_00424C40);

INCLUDE_ASM("asm/levels", func_L03_00425030);

INCLUDE_ASM("asm/levels", func_L03_00425108);

INCLUDE_ASM("asm/levels", func_L03_0042B6F8);

INCLUDE_ASM("asm/levels", func_L03_0042B868);

INCLUDE_ASM("asm/levels", func_L03_0042B908);

INCLUDE_ASM("asm/levels", func_L03_0042B930);

INCLUDE_ASM("asm/levels", func_L03_0043AC40);

INCLUDE_ASM("asm/levels", func_L03_0043AC78);
extern void func_L03_0043ACD8(void);
INCLUDE_ASM("asm/levels", func_L03_0043ACD8);


INCLUDE_ASM("asm/levels", func_L03_0043AD30);

INCLUDE_ASM("asm/levels", func_L03_0043AF30);

INCLUDE_ASM("asm/levels", func_L03_00454888);

void func_L03_00456E58(void)
{
}

void func_L03_004586F0(void)
{
}
