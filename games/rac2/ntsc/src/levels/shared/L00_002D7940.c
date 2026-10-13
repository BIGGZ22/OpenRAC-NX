#include "include_asm.h"

typedef int s32;

extern float func_00283180_13(float x);

extern float func_00283198_14(float x);

extern void func_00282CC0_42(float *out, float *in, void *q);

typedef unsigned char u8;

typedef float f32;

extern void func_002B00D8(char *a, char *b, char *c, f32 d);

extern void func_00282CC0_5(char *a, char *b, char *c);

extern void func_00283098_1(char *a, char *b, char *c);

extern void func_00283958(char *a, char *b);

extern void func_00283738(char *a, char *b, char *c);

extern void func_00282CF0_4(char *a, char *b, char *c);

#ifndef RAC2_T_V4_F4778F810
#define RAC2_T_V4_F4778F810
typedef struct {
    f32 x, y, z, w;
} V4_F4778f810;
#endif

#ifndef RAC2_T_SUB_F4778F810
#define RAC2_T_SUB_F4778F810
typedef struct {
    u8 pad000[0x10];
    u8 at010[0x10];
    u8 at020[0x28];
    f32 f048;
} Sub_F4778f810;
#endif

#ifndef RAC2_T_OBJ_F4778F810
#define RAC2_T_OBJ_F4778F810
typedef struct {
    u8 pad000[0x10];
    u8 at010[0x58];
    char *p068;
    u8 pad06C[0xC0 - 0x6C];
    u8 at0C0[0x10];
} Obj_F4778f810;
#endif

/* clear five object fields; independently qualified in this overlay */
void func_L00_002D7940(s32 *object) {
    object[0]=0; object[1]=0; object[2]=0; object[3]=0; object[4]=0;
}

INCLUDE_ASM("asm/levels", func_L00_002D7958);

INCLUDE_ASM("asm/levels", func_L00_002D7D58);

INCLUDE_ASM("asm/levels", func_L00_002D8208);

typedef struct {
    int key;
    int pad[4];
} E_r002D9D48;

extern E_r002D9D48 r1_D_L00_001EAC00[];

/* rac1-decomp src/overlays/shared/camera_001EB508.c: func_L00_001ED5B0 */
/* Returns the index of the entry with the given key, or of the -1 terminator. */
int func_L00_002D9D48(int key) {
    int i = 0;
    while (r1_D_L00_001EAC00[i].key != -1 && r1_D_L00_001EAC00[i].key != key) {
        i++;
    }
    return i;
}

INCLUDE_ASM("asm/levels", func_L00_002D9D98);

INCLUDE_ASM("asm/levels", func_L00_002D9E80);

/* Builds a 3-component vector from a spherical radius/elevation/azimuth triple using two unary float helpers, clears the 4th component, then forwards the destination twice plus a third argument to a final helper. */
void func_L00_002DA0B8(float *out, float *p, void *q)
{
    float t;

    t = func_00283180_13(p[1]);
    out[0] = p[2] * t * func_00283180_13(p[0]);
    t = func_00283180_13(p[1]);
    out[1] = p[2] * t * func_00283198_14(p[0]);
    out[2] = p[2] * func_00283198_14(p[1]);
    out[3] = 0.0f;
    func_00282CC0_42(out, out, q);
}

INCLUDE_ASM("asm/levels", func_L00_002DA1E0);

INCLUDE_ASM("asm/levels", func_L00_002DA390);

INCLUDE_ASM("asm/levels", func_L00_002DB350);

INCLUDE_ASM("asm/levels", func_L00_002DB4B8);

/* Return the constant signed integer one. */
/* Returns the constant integer 1. Matches FUN_0012F940 disassembly and decompilation. */
int func_L00_002DC3D0(void) {
    return 1;
}

/* Return the constant signed integer one. */
/* Returns the constant integer 1. Matches FUN_0012F940 disassembly and decompilation. */
int func_L00_002DC3D8(void) {
    return 1;
}

/* Drives eight calls over a parameter object: loads the pointer at +104, builds four 16-byte stack scratch records at sp+0/+16/+32/+48, and passes the object's +16 and +192 regions to the callees. */
void func_L00_002DC3E0(Obj_F4778f810 *obj)
{
    V4_F4778f810 tmp[6];
    Sub_F4778f810 *p = (Sub_F4778f810 *)obj->p068;

    func_002B00D8((char *)obj + 0x10, (char *)p, (char *)p, p->f048);
    func_00282CC0_5((char *)obj + 0x10, (char *)obj + 0x10, (char *)p);
    func_00283098_1((char *)&tmp[0], (char *)p + 0x20, (char *)obj + 0xC0);
    func_00283958((char *)p + 0x10, (char *)&tmp[3]);
    func_00283738((char *)obj + 0xC0, (char *)&tmp[3], (char *)obj + 0xC0);
    func_00283098_1((char *)&tmp[1], (char *)p + 0x20, (char *)obj + 0xC0);
    func_00282CF0_4((char *)&tmp[2], (char *)&tmp[1], (char *)&tmp[0]);
    func_00282CF0_4((char *)obj + 0x10, (char *)obj + 0x10, (char *)&tmp[2]);
}

/* Drives eight calls over a parameter object: loads the pointer at +104, builds four 16-byte stack scratch records at sp+0/+16/+32/+48, and passes the object's +16 and +192 regions to the callees. */
void func_L00_002DC4C0(Obj_F4778f810 *obj)
{
    V4_F4778f810 tmp[6];
    Sub_F4778f810 *p = (Sub_F4778f810 *)obj->p068;

    func_002B00D8((char *)obj + 0x10, (char *)p, (char *)p, p->f048);
    func_00282CC0_5((char *)obj + 0x10, (char *)obj + 0x10, (char *)p);
    func_00283098_1((char *)&tmp[0], (char *)p + 0x20, (char *)obj + 0xC0);
    func_00283958((char *)p + 0x10, (char *)&tmp[3]);
    func_00283738((char *)obj + 0xC0, (char *)&tmp[3], (char *)obj + 0xC0);
    func_00283098_1((char *)&tmp[1], (char *)p + 0x20, (char *)obj + 0xC0);
    func_00282CF0_4((char *)&tmp[2], (char *)&tmp[1], (char *)&tmp[0]);
    func_00282CF0_4((char *)obj + 0x10, (char *)obj + 0x10, (char *)&tmp[2]);
}

INCLUDE_ASM("asm/levels", func_L00_002E0200);

INCLUDE_ASM("asm/levels", func_L00_002E2048);

INCLUDE_ASM("asm/levels", func_L00_002E2958);

INCLUDE_ASM("asm/levels", func_L00_002E2C30);

INCLUDE_ASM("asm/levels", func_L00_002E30B8);

INCLUDE_ASM("asm/levels", func_L00_002E3340);

INCLUDE_ASM("asm/levels", func_L00_002E3520);

INCLUDE_ASM("asm/levels", func_L00_002E36F8);

INCLUDE_ASM("asm/levels", func_L00_002E3898);

/* evaluates (b-a)*(t*t)+a*t and converts the measured float result to a signed word */
s32 func_L00_002E3A68(f32 a,f32 b,f32 t) {
 f32 difference=b-a;
 f32 squared=t*t;
 a=a*t;
 difference=difference*squared;
 a=a+difference;
 return (s32)a;
}

INCLUDE_ASM("asm/levels", func_L00_002E3A90);
