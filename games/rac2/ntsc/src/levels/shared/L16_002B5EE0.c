#include "include_asm.h"

typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef signed int s32;
typedef unsigned int u32;
typedef float f32;

typedef int TI128 __attribute__((mode(TI)));

INCLUDE_ASM("asm/levels", func_L16_002B5EE0);

INCLUDE_ASM("asm/levels", func_L16_002E5440);

INCLUDE_ASM("asm/levels", func_L16_00395250);

INCLUDE_ASM("asm/levels", func_L16_00395858);

INCLUDE_ASM("asm/levels", func_L16_003959B8);

extern char D_00189E20[];
extern void func_002AFDD8(char *object, void *out, void *table, int a3);
extern float func_00283248(float a, float b);
extern float func_00282C48(float a);
extern void func_00282F20(char *dst, char *src, float x);
extern float func_00283198(float a);
extern float func_00283180(float a);
extern void func_00283958(char *a, char *b);
extern void func_00283738(char *a, char *b, char *c);

/* steers the object along the ground normal the terrain query returns */
INCLUDE_ASM("asm/levels", func_L16_00395C00);

INCLUDE_ASM("asm/levels", func_L16_00395D30);

INCLUDE_ASM("asm/levels", func_L16_00395ED0);

/* Read and return the float at linked offset 0x 34 through the pointer stored at owner+0x 68. */
float func_L16_003960E8(const unsigned char *owner)
{
    const unsigned char *linked = *(const unsigned char * const *)(owner + 0x68);
    return *(const float *)(linked + 0x34);
}

extern int func_002DA3C8(void *v, int a1, char *object, int a3, float f);
extern char *D_L16_001C3FC0[];

/* reports whether the first live marker in the run still carries the kind byte it looks for */
INCLUDE_ASM("asm/levels", func_L16_003960F8);

INCLUDE_ASM("asm/levels", func_L16_003A8D58);

INCLUDE_ASM("asm/levels", func_L16_003A90E0);

INCLUDE_ASM("asm/levels", func_L16_003C2C08);

INCLUDE_ASM("asm/levels", func_L16_003C32F8);

INCLUDE_ASM("asm/levels", func_L16_003C3870);

INCLUDE_ASM("asm/levels", func_L16_003C3A10);

INCLUDE_ASM("asm/levels", func_L16_004051A8);

INCLUDE_ASM("asm/levels", func_L16_00405818);

INCLUDE_ASM("asm/levels", func_L16_00406080);

INCLUDE_ASM("asm/levels", func_L16_00406550);

INCLUDE_ASM("asm/levels", func_L16_0040BED0);

INCLUDE_ASM("asm/levels", func_L16_0040C5F0);

extern char *D_L16_001B2C0C __attribute__((sda));
extern char *D_L16_001B2B1C __attribute__((sda));

/* copies the 16-byte record the object's two indices pick out of the chosen table into dst */
INCLUDE_ASM("asm/levels", func_L16_0040C7E8);

INCLUDE_ASM("asm/levels", func_L16_00413850);

INCLUDE_ASM("asm/levels", func_L16_00414038);

INCLUDE_ASM("asm/levels", func_L16_0041F950);

extern char *D_L16_001B9B50;

/* pushes the two axis values onto the linked node, unless the node is inert or a value is zero */
void func_L16_0042FB58(float x, float y)
{
    char *st = D_L16_001B9B50;
    char *p;

    if (*(s16 *)(st + 0x86) != 0)
        return;
    p = *(char **)(st + 0x70) + 0x60;
    if (x != 0.0f)
        *(float *)(p + 0xE4) = x;
    if (y != 0.0f)
        *(float *)(p + 0xE8) = y;
}

extern void func_L16_00435CE8(void);

/* runs the level's one-shot start-up; the caller uses this only for its side effects */
int func_L16_004364B0(void)
{
    func_L16_00435CE8();
    return 0;
}
