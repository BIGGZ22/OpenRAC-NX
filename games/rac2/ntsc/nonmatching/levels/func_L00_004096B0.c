#include "include_asm.h"

extern int func_00282978_5(char *handle);

extern void func_0029F338_2(char *object);

#ifndef RAC2_T_NODE16_F23626AD8
#define RAC2_T_NODE16_F23626AD8
typedef struct Node16_F23626ad8 Node16_F23626ad8;
#endif

#ifndef RAC2_T_NODE104_F23626AD8
#define RAC2_T_NODE104_F23626AD8
typedef struct Node104_F23626ad8 Node104_F23626ad8;
#endif

#ifndef RAC2_T_NODE24_F23626AD8
#define RAC2_T_NODE24_F23626AD8
typedef struct Node24_F23626ad8 Node24_F23626ad8;
#endif

struct Node16_F23626ad8 {
    short h0;
    unsigned char pad2[2];
    unsigned char b4;
    unsigned char b5;
    unsigned char b6;
};

struct Node104_F23626ad8 {
    unsigned char pad0[16];
    Node16_F23626ad8 *p16;
};

struct Node24_F23626ad8 {
    unsigned char pad0[52];
    unsigned short h52;
    unsigned char pad54[50];
    Node104_F23626ad8 *p104;
};

#ifndef RAC2_T_OBJ_F23626AD8
#define RAC2_T_OBJ_F23626AD8
typedef struct Obj_F23626ad8 {
    unsigned char pad0[4];
    union {
        int f4;
        struct {
            unsigned char b4;
            unsigned char b5;
            short h6;
        } b;
    } u;
    unsigned char pad8[16];
    Node24_F23626ad8 *f24;
} Obj_F23626ad8;
#endif

extern void func_002A0488(Node24_F23626ad8 *a, unsigned char b, unsigned char c, unsigned char d);

extern void func_00282CF0_9(char *local, char *first, char *second);

extern void func_00282CC0_15(char *target, char *first, char *local);

#ifndef RAC2_T_VEC16_FC936841D
#define RAC2_T_VEC16_FC936841D
typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec16_Fc936841d;
#endif

void func_L00_004096B0(void)
{
}
